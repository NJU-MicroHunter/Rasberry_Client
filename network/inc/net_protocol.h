#ifndef RASBERRY_CLIENT_NET_PROTOCOL_H
#define RASBERRY_CLIENT_NET_PROTOCOL_H

#define NET_PROTOCOL_NUM 3

#define NET_EMPTY_NUM ((unsigned char)0)
#define NET_EMPTY_MAX 1
#define NET_HELLO_NUM ((unsigned char)1)
#define NET_HELLO_MAX 1
#define NET_IMAGE_NUM ((unsigned char)2)
#define NET_IMAGE_MAX 5

#define NET_PROTOCOL_MAX (NET_EMPTY_MAX + NET_HELLO_MAX + NET_IMAGE_MAX)


#endif //RASBERRY_CLIENT_NET_PROTOCOL_H
