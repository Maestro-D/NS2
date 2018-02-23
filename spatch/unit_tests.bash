#!/bin/bash

# Unit tests

ssh -p 55022 mr-spatch@localhost
ssh -p 55022 mr-spatch@localhost -t "ls -la"
