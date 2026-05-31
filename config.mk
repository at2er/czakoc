CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-parameter -D_DEFAULT_SOURCE -pedantic -std=c99 \
	 -Ilibmcb/include -Iinclude
LDFLAGS = -lmcb -Llibmcb
AR = ar
PREFIX = /usr/local
