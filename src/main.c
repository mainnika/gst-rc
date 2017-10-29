#include <stdio.h>
#include <gst/gst.h>

typedef struct {
	GstPipeline *pipeline;
	GstElement *src;
	GstElement *decoder;
	GstElement *convert;
	GstElement *sink;
	GMainLoop *loop;
} gst_rc_t;

static gst_rc_t gst_rc;

static void on_pad_added(GstElement *element, GstPad *pad)
{
	GstCaps *caps;
	GstStructure *str;
	gchar *name;
	GstPad *convert;
	GstPadLinkReturn ret;

	g_debug("pad added");

	caps = gst_pad_get_current_caps(pad);
	str = gst_caps_get_structure(caps, 0);

	g_assert(str);

	name = (gchar*) gst_structure_get_name(str);

	g_debug("pad name %s", name);

	if (g_strrstr(name, "video")) {
		convert = gst_element_get_static_pad(gst_rc.convert, "sink");
		g_assert(convert);
		ret = gst_pad_link(pad, convert);
		g_debug("pad_link returned %d\n", ret);
		gst_object_unref(convert);
	}

	gst_caps_unref(caps);
}

static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer *ptr)
{
	gst_rc_t *app = (gst_rc_t*) ptr;

	switch (GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR:
	{
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		g_print("Error %s\n", err->message);
		g_error_free(err);
		g_free(debug);
		g_main_loop_quit(app->loop);
	}
		break;

	case GST_MESSAGE_EOS:
		g_print("End of stream\n");
		g_main_loop_quit(app->loop);
		break;

	default:
		break;
	}

	return TRUE;
}

static gboolean cb_rewind(GstElement *pipeline)
{
	gint64 pos, len;

	if (gst_element_query_position(pipeline, GST_FORMAT_TIME, &pos) &&
		gst_element_query_duration(pipeline, GST_FORMAT_TIME, &len)) {

		g_debug("Rewind at %"GST_TIME_FORMAT" / %"GST_TIME_FORMAT, GST_TIME_ARGS(pos), GST_TIME_ARGS(len));
	}

	if (!gst_element_seek(pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {

		g_debug("Rewind failed!");
	}

	return TRUE;
}

int main(int argc, char *argv[])
{
	gst_rc_t *app = &gst_rc;
	GstBus *bus;
	GstStateChangeReturn state_ret;

	if (argc != 2) {
		printf("File name not specified\n");
		return 1;
	}

	gst_init(NULL, NULL);

	app->pipeline = (GstPipeline*) gst_pipeline_new("pipeline");
	bus = gst_pipeline_get_bus(app->pipeline);
	gst_bus_add_watch(bus, (GstBusFunc) bus_callback, app);
	gst_object_unref(bus);

	app->src = gst_element_factory_make("filesrc", "src");
	app->decoder = gst_element_factory_make("decodebin", "decoder");
	app->convert = gst_element_factory_make("videoconvert", "convert");
	app->sink = gst_element_factory_make("aasink", "sink");

	g_assert(app->src);
	g_assert(app->decoder);
	g_assert(app->convert);
	g_assert(app->sink);

	g_signal_connect(app->decoder, "pad-added", G_CALLBACK(on_pad_added), app->decoder);
	g_object_set(G_OBJECT(app->src), "location", argv[1], NULL);

	gst_bin_add_many(GST_BIN(app->pipeline), app->src, app->decoder, app->convert, app->sink, NULL);

	if (!gst_element_link((GstElement*) app->src, app->decoder)) {
		g_warning("failed to link src and decoder");
	}

	if (!gst_element_link(app->convert, app->sink)) {
		g_warning("failed to link converter and sink");
	}

	state_ret = gst_element_set_state((GstElement*) app->pipeline, GST_STATE_PLAYING);
	g_warning("set state returned %d\n", state_ret);

	app->loop = g_main_loop_new(NULL, FALSE);
	printf("Running main loop\n");

	g_timeout_add(2000, (GSourceFunc) cb_rewind, app->pipeline);
	g_main_loop_run(app->loop);

	state_ret = gst_element_set_state((GstElement*) app->pipeline, GST_STATE_NULL);
	g_warning("set state null returned %d\n", state_ret);

	return 0;
}
