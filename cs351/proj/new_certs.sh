#!/bin/bash

# Ensure needed directories exist.
mkdir -p .certs/.old

# Move any existing certs to old.
# TODO: Remove this after done with dev phase.
if (( "$(ls -1 .certs |wc -l)" > 0 )); then
  for file in .certs/*; do
    mv "${file}" ".certs/.old/${file##*/}.$(date -Is)"
  done
fi

# Create a new key pair according to our policy:
# RSA 4096 + SHA-256, no DES, self-signed (localhost), live for 1 year.
openssl req -x509 -newkey rsa:4096 -sha256 \
  -keyout .certs/key.pem -out .certs/cert.pem -days 365 \
  -nodes -subj='/CN=localhost'
