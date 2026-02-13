/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_rpc_tr.h>
#include <nrf_rpc_errno.h>

#include <zephyr/posix/sys/socket.h>
#include <zephyr/posix/unistd.h>
#include <zephyr/posix/fcntl.h>
#include <errno.h>
#include <string.h>

/* sockaddr_un is defined in Zephyr's net headers */
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(nrf_rpc_socket, CONFIG_NRF_RPC_TR_LOG_LEVEL);

#define SOCKET_PATH "/tmp/nrf_rpc_server.sock"
#define MAX_PACKET_SIZE 1536

/* Socket transport context */
struct nrf_rpc_socket {
	int server_fd;
	int client_fd;
	nrf_rpc_tr_receive_handler_t receive_callback;
	void *callback_context;
	const struct nrf_rpc_tr *transport;
	struct k_thread rx_thread;
};

static K_THREAD_STACK_DEFINE(rx_thread_stack, 2048);

static struct nrf_rpc_socket socket_ctx = {
	.server_fd = -1,
	.client_fd = -1,
};

static void rx_thread(void *arg1, void *arg2, void *arg3)
{
	struct nrf_rpc_socket *ctx = arg1;
	uint8_t buffer[MAX_PACKET_SIZE];
	ssize_t received;

	LOG_INF("RX thread started, waiting for client connection");

	/* Accept connection */
	while (ctx->client_fd < 0) {
		ctx->client_fd = accept(ctx->server_fd, NULL, NULL);
		if (ctx->client_fd < 0) {
			k_sleep(K_MSEC(100));
		} else {
			LOG_INF("Client connected on socket");
			/* Set non-blocking */
			fcntl(ctx->client_fd, F_SETFL, O_NONBLOCK);
		}
	}

	while (true) {
		received = recv(ctx->client_fd, buffer, sizeof(buffer), 0);

		if (received > 0) {
			LOG_DBG("Received %d bytes from socket", (int)received);
			if (ctx->receive_callback) {
				ctx->receive_callback(ctx->transport, buffer, 
						      received, ctx->callback_context);
			}
		} else if (received == 0) {
			LOG_INF("Client disconnected");
			close(ctx->client_fd);
			ctx->client_fd = -1;

			/* Wait for new connection */
			while (ctx->client_fd < 0) {
				ctx->client_fd = accept(ctx->server_fd, NULL, NULL);
				if (ctx->client_fd >= 0) {
					LOG_INF("Client reconnected");
					fcntl(ctx->client_fd, F_SETFL, O_NONBLOCK);
				} else {
					k_sleep(K_MSEC(100));
				}
			}
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG_ERR("recv error: %d", errno);
		}

		k_sleep(K_MSEC(1));
	}
}

static int init(const struct nrf_rpc_tr *transport, 
		nrf_rpc_tr_receive_handler_t callback, void *context)
{
	struct nrf_rpc_socket *ctx = transport->ctx;
	struct sockaddr_un addr;
	int ret;

	LOG_INF("Initializing nRF RPC socket transport on %s", SOCKET_PATH);

	ctx->transport = transport;
	ctx->receive_callback = callback;
	ctx->callback_context = context;

	/* Create socket */
	LOG_DBG("Calling socket(AF_UNIX=%d, SOCK_STREAM=%d, 0)", AF_UNIX, SOCK_STREAM);
	ctx->server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	LOG_DBG("socket() returned: %d, errno: %d", ctx->server_fd, errno);
	if (ctx->server_fd < 0) {
		LOG_ERR("Failed to create socket: fd=%d, errno=%d", ctx->server_fd, errno);
		return -NRF_EFAULT;
	}

	/* Remove old socket file if it exists */
	unlink(SOCKET_PATH);

	/* Bind socket */
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

	ret = bind(ctx->server_fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		LOG_ERR("Failed to bind socket: %d", errno);
		close(ctx->server_fd);
		return -NRF_EFAULT;
	}

	/* Listen for connections */
	ret = listen(ctx->server_fd, 1);
	if (ret < 0) {
		LOG_ERR("Failed to listen: %d", errno);
		close(ctx->server_fd);
		return -NRF_EFAULT;
	}

	/* Set non-blocking for accept */
	fcntl(ctx->server_fd, F_SETFL, O_NONBLOCK);

	LOG_INF("Socket transport listening on %s", SOCKET_PATH);

	/* Start RX thread */
	k_thread_create(&ctx->rx_thread, rx_thread_stack,
			K_THREAD_STACK_SIZEOF(rx_thread_stack),
			rx_thread, ctx, NULL, NULL,
			K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
	k_thread_name_set(&ctx->rx_thread, "nrf_rpc_rx");

	return 0;
}

static int socket_send(const struct nrf_rpc_tr *transport, const uint8_t *data, size_t length)
{
	struct nrf_rpc_socket *ctx = transport->ctx;
	ssize_t sent;

	if (ctx->client_fd < 0) {
		LOG_WRN("No client connected, dropping packet");
		return -NRF_EAGAIN;
	}

	sent = send(ctx->client_fd, data, length, 0);
	if (sent < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -NRF_EAGAIN;
		}
		LOG_ERR("send error: %d", errno);
		return -NRF_EIO;
	}

	if (sent != length) {
		LOG_WRN("Partial send: %d/%zu", (int)sent, length);
		return -NRF_EIO;
	}

	LOG_DBG("Sent %zu bytes to socket", length);
	return 0;
}

static void *tx_buf_alloc(const struct nrf_rpc_tr *transport, size_t *size)
{
	void *data = NULL;

	data = k_malloc(*size);
	if (!data) {
		LOG_ERR("Failed to allocate TX buffer");
		k_oops();
		*size = 0;
		return NULL;
	}

	return data;
}

static void tx_buf_free(const struct nrf_rpc_tr *transport, void *buf)
{
	k_free(buf);
}

/* Transport API implementation */
const struct nrf_rpc_tr_api nrf_rpc_socket_api = {
	.init = init,
	.send = socket_send,
	.tx_buf_alloc = tx_buf_alloc,
	.tx_buf_free = tx_buf_free,
};

/* Transport instance */
const struct nrf_rpc_tr nrf_rpc_socket_transport = {
	.api = &nrf_rpc_socket_api,
	.ctx = &socket_ctx,
};

/* Make the transport globally accessible with a well-known name */
extern const struct nrf_rpc_tr *nrf_rpc_primary_transport(void);
const struct nrf_rpc_tr *nrf_rpc_primary_transport(void)
{
	return &nrf_rpc_socket_transport;
}
