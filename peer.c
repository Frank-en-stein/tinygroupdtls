/* dtls -- a very basic DTLS implementation
 *
 * Copyright (C) 2011--2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "peer.h"
#include "debug.h"

#ifndef WITH_CONTIKI
void peer_init()
{
}

static inline dtls_peer_t *
dtls_malloc_peer() {
  return (dtls_peer_t *)malloc(sizeof(dtls_peer_t));
}

void
dtls_free_peer(dtls_peer_t *peer) {
  dtls_cipher_free(peer->security_params.read_cipher);
  dtls_cipher_free(peer->security_params.write_cipher);

  free(peer);
}
#else /* WITH_CONTIKI */

#include "memb.h"
MEMB(peer_storage, dtls_peer_t, DTLS_PEER_MAX);

void
peer_init() {
  memb_init(&peer_storage);
}

static inline dtls_peer_t *
dtls_malloc_peer() {
  return memb_alloc(&peer_storage);
}

void
dtls_free_peer(dtls_peer_t *peer) {
  dtls_cipher_free(peer->security_params.read_cipher);
  dtls_cipher_free(peer->security_params.write_cipher);

  memb_free(&peer_storage, peer);
}
#endif // WITH_CONTIKI


dtls_peer_t *
dtls_new_peer(const session_t *session) {
  dtls_peer_t *peer;

  peer = dtls_malloc_peer();
  if (peer) {
    memset(peer, 0, sizeof(dtls_peer_t));
    memcpy(&peer->session, session, sizeof(session_t));

    dtls_dsrv_log_addr(LOG_DEBUG, "dtls_new_peer", session);
    /* initially allow the NULL cipher */
    peer->security_params.cipher = TLS_NULL_WITH_NULL_NULL;
    peer->security_params.compression = TLS_COMPRESSION_NULL;

    /* initialize the handshake hash wrt. the hard-coded DTLS version */
    debug("DTLSv12: initialize HASH_SHA256\n");
    /* TLS 1.2:  PRF(secret, label, seed) = P_<hash>(secret, label + seed) */
    /* FIXME: we use the default SHA256 here, might need to support other
              hash functions as well */
    dtls_hash_init(&peer->hs_state.hs_hash);
  }
  else
  {
    debug("ERROR: Insufficient memory to create new peer\r\n");
  }

  return peer;
}
