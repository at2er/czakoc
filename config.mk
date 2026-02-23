CC = gcc
CFLAGS = -Wall -Wextra -D_DEFAULT_SOURCE -pedantic -std=c99 \
	 -Ilibmcb/include
LDFLAGS = -Llibmcb -lmcb
AR = ar
PREFIX = /usr/local
