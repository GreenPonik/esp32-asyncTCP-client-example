#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <WiFi.h>

#ifndef CONFIG_H
#define CONFIG_H

#define SSID "esp-tcp-server"
#define PASSWORD "123456789"

#define SERVER_HOST_NAME "esp-tcp-client"

#define TCP_SERVER_PORT 8000
#define DNS_PORT 53

#endif // CONFIG_H

static DNSServer DNS;

void replyToServer(void *arg)
{
	AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
	//our big json string test
	String jsonString = "{\"data_from_module_type\":1,\"hub_unique_id\":\"hub-bfd\",\"slave_unique_id\":\"water-sensor-ba9\",\"water_sensor\":{\"unique_id\":\"water-sensor-ba9\",\"firmware\":\"0.0.1\",\"hub_unique_id\":\"hub-bfd\",\"ip_address\":\"192.168.4.2\",\"mdns\":\"water-sensor-ba9.local\",\"pair_status\":254,\"ec\":{\"value\":0,\"calib_launch\":0,\"sensor_k_origin\":1,\"sensor_k_calibration\":1,\"calibration_solution\":1,\"regulation_state\":1,\"max_pumps_durations\":5000,\"set_point\":200},\"ph\":{\"value\":0,\"calib_launch\":0,\"regulation_state\":1,\"max_pumps_durations\":5000,\"set_point\":700},\"water\":{\"temperature\":0,\"pump_enable\":false}}}";
	// send reply
	if (client->space() > strlen(jsonString.c_str()) && client->canSend())
	{
		client->add(jsonString.c_str(), strlen(jsonString.c_str()));
		client->send();
	}
}

void handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
	Serial.printf("\n data received from %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t *)data, len);
}

void onConnect(void *arg, AsyncClient *client)
{
	Serial.printf("\n client has been connected to %s on port %d \n", SERVER_HOST_NAME, TCP_SERVER_PORT);
	replyToServer(client);
}
AsyncClient *client_tcp = new AsyncClient;

void setup()
{
	Serial.begin(115200);
	// connects to access point
	WiFi.mode(WIFI_STA);
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(500);
	}
	client_tcp->onData(handleData, client_tcp);
	client_tcp->onConnect(onConnect, client_tcp);

	// start dns server
	if (!DNS.start(DNS_PORT, SERVER_HOST_NAME, WiFi.softAPIP()))
		Serial.printf("\n failed to start dns service \n");
}

void loop()
{
	DNS.processNextRequest();
	delay(2000);
	Serial.println("");
	client_tcp->connect("192.168.4.1", TCP_SERVER_PORT);
	client_tcp->close();
}