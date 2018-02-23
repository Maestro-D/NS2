#!/bin/bash

OUTPUT_DIR="./host_keys"

# Create output directory if not exists
mkdir -p "$OUTPUT_DIR"

# Create dsa, ecdsa, ed25519 and rsa host keys
ssh-keygen -f "$OUTPUT_DIR/ssh_host_dsa_key" -N "" -t dsa
ssh-keygen -f "$OUTPUT_DIR/ssh_host_ecdsa_key" -N "" -t ecdsa
ssh-keygen -f "$OUTPUT_DIR/ssh_host_ed25519_key" -N "" -t ed25519
ssh-keygen -f "$OUTPUT_DIR/ssh_host_rsa_key" -N "" -t rsa
