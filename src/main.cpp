#include "pipeline.h"
#include "jsonrpc.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
	auto pipeline = std::make_shared<Pipeline>();

	if (argc != 2) {
		printf("File name not specified\n");
		return 1;
	}

	pipeline->create_element("src", "filesrc", {{"location", argv[1]}});
	pipeline->create_element("decoder", "decodebin");
	pipeline->create_element("converter", "videoconvert");
	pipeline->create_element("sink", "aasink");
	
	pipeline->create_relation("src", "decoder", Pipeline::RelationType::Simple);
	pipeline->create_relation("decoder", "converter", Pipeline::RelationType::Pad, {"video", "sink"});
	pipeline->create_relation("converter", "sink", Pipeline::RelationType::Simple);
	
	pipeline->set_state(GST_STATE_PLAYING);
	pipeline->run();
	pipeline->set_state(GST_STATE_NULL);
	
	return 0;
}
