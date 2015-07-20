#include "incs.h"

#include <openssl/hmac.h>

/*
 * Message-Authenticator attribute
 */

void test23(void)
{
	RADIUS_PACKET *packet;
	RADIUS_PACKET *response;
	HMAC_CTX ctx;

	uint8_t packetdata[] = {
		RADIUS_CODE_ACCESS_REQUEST, 0x7f, 0, 48,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* auth */
		10, 10, 'h', 'o', 'g', 'e', 'f', 'u', 'g', 'a',
		RADIUS_TYPE_MESSAGE_AUTHENTICATOR, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	uint8_t responsedata[] = {
		RADIUS_CODE_ACCESS_ACCEPT, 0x7f, 0, 49,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* auth */
		10, 11, 'f', 'o', 'o', 'b', 'a', 'r', 'b', 'a', 'z',
		RADIUS_TYPE_MESSAGE_AUTHENTICATOR, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	packet = radius_new_request_packet(RADIUS_CODE_ACCESS_REQUEST);
	radius_set_id(packet, 0x7f);
	radius_put_string_attr(packet, 10, "hogefuga");
	radius_put_message_authenticator(packet, "sharedsecret");

	radius_get_authenticator(packet, packetdata + 4);
	HMAC(EVP_md5(), "sharedsecret", 12, packetdata, sizeof(packetdata), packetdata + sizeof(packetdata) - 16, NULL);

	CHECK(radius_get_length(packet) == sizeof(packetdata));
	CHECK(memcmp(radius_get_data(packet), packetdata, sizeof(packetdata)) == 0);
	CHECK(radius_check_message_authenticator(packet, "sharedsecret") == 0);

	response = radius_new_response_packet(RADIUS_CODE_ACCESS_ACCEPT, packet);
	radius_put_string_attr(response, 10, "foobarbaz");
	radius_put_message_authenticator(response, "sharedsecret");

	radius_get_authenticator(response, responsedata + 4);
	HMAC_Init(&ctx, "sharedsecret", 12, EVP_md5());
	HMAC_Update(&ctx, responsedata, 4);
	HMAC_Update(&ctx, packetdata + 4, 16);
	HMAC_Update(&ctx, responsedata + 20, sizeof(responsedata) - 20);
	HMAC_Final(&ctx, responsedata + sizeof(responsedata) - 16, NULL);
	HMAC_cleanup(&ctx);

	CHECK(radius_get_length(response) == sizeof(responsedata));
	CHECK(memcmp(radius_get_data(response), responsedata, sizeof(responsedata)) == 0);
	CHECK(radius_check_message_authenticator(response, "sharedsecret") == 0);

	radius_set_raw_attr(packet, 10, "hogefuge", 8);
	CHECK(radius_check_message_authenticator(packet, "sharedsecret") != 0);
	radius_set_raw_attr(response, 10, "zapzapzap", 9);
	CHECK(radius_check_message_authenticator(response, "sharedsecret") != 0);

	radius_set_raw_attr(packet, 10, "hogefuga", 8);
	radius_set_id(packet, 0xff);
	radius_set_message_authenticator(packet, "sharedsecret");
	packetdata[1] = 0xff;
	memset(packetdata + sizeof(packetdata) - 16, 0, 16);
	HMAC(EVP_md5(), "sharedsecret", 12, packetdata, sizeof(packetdata), packetdata + sizeof(packetdata) - 16, NULL);
	CHECK(memcmp(radius_get_data(packet), packetdata, sizeof(packetdata)) == 0);
}

ADD_TEST(test23)
