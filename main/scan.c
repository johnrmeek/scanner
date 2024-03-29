/* Scan Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"

#define	LED_GPIO_PIN GPIO_NUM_4
#define	WIFI_CHANNEL_MAX 13
#define	WIFI_CHANNEL_SWITCH_INTERVAL 500

static wifi_country_t wifi_country = {.cc="US", .schan=1, .nchan=13, .policy=WIFI_COUNTRY_POLICY_AUTO};

typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static void wifi_sniffer_init(void);
static void wifi_sniffer_set_channel(uint8_t channel);
static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

void app_main(void)
{
	uint8_t channel = 1;

	/* setup */
	wifi_sniffer_init();

	/* loop */
    // Add code to recieve IO input to cancel and exit routine... 
	while (true) {
		vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
		wifi_sniffer_set_channel(channel);
		channel = (channel % WIFI_CHANNEL_MAX) + 1;

		if (channel == 1)
		{
       		printf("Scanned from 1 to %d\n", WIFI_CHANNEL_MAX);			
	        vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
    }
    fflush(stdout);
    //esp_restart();
}

void wifi_sniffer_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 13] */
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
    ESP_ERROR_CHECK( esp_wifi_start() );
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

void wifi_sniffer_set_channel(uint8_t channel)
{
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

const char * wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
	switch(type) {
		case WIFI_PKT_MGMT: return "MGMT";
		case WIFI_PKT_DATA: return "DATA";
		default:	
		case WIFI_PKT_MISC: return "MISC";
	}
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{

	if (type != WIFI_PKT_MGMT)
		return;

	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    //uint8_t addr1[6]; /* receiver address */
	//uint8_t addr2[6]; /* sender address */
	//uint8_t addr3[6]; /* filtering address */

	uint8_t broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	if (memcmp(hdr->addr1, broadcast_address, sizeof(broadcast_address)) == 0)
	{
		uint8_t sender_address[6] = {0x8A, 0xEE, 0x13, 0x57, 0x61, 0xAE};

		if (memcmp(hdr->addr2, sender_address, sizeof(sender_address)) == 0)
		{
			printf("MATCHED SENDER - PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
				" ADDR2=%02x:%02x:%02x:%02x:%02x:%02x\n",
				wifi_sniffer_packet_type2str(type),
				ppkt->rx_ctrl.channel,
				ppkt->rx_ctrl.rssi,
				/* ADDR2 */
				hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
				hdr->addr2[3],hdr->addr2[4],hdr->addr2[5]
			);
		}
		else {
			// printf("UNMATCHED SENDER - PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
			// 	" ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
			// 	" ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
			// 	wifi_sniffer_packet_type2str(type),
			// 	ppkt->rx_ctrl.channel,
			// 	ppkt->rx_ctrl.rssi,
			// 	/* ADDR2 */
			// 	hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
			// 	hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
			// 	/* ADDR3 */
			// 	hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
			// 	hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
			// );
		}
	}
	else
	{
			// printf("UNKNOWN RECIEVER - PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
			// 	" ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
			// 	" ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
			// 	" ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
			// 	wifi_sniffer_packet_type2str(type),
			// 	ppkt->rx_ctrl.channel,
			// 	ppkt->rx_ctrl.rssi,
			// 	/* ADDR1 */
			// 	hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
			// 	hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
			// 	/* ADDR2 */
			// 	hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
			// 	hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
			// 	/* ADDR3 */
			// 	hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
			// 	hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
			// );
	}
}




