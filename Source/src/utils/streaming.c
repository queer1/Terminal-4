#include "streaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>
#include <glib.h>

#define gst_element_default_deep_notify gst_object_default_deep_notify

static GMainLoop *loop;

gboolean media_link_elements_with_filter(GstElement *v4l2src,
		GstElement *dmaiaccel) {
	gboolean link_ok;
	GstCaps *caps;

	caps = gst_caps_new_simple("video/x-raw-yuv", "format", GST_TYPE_FOURCC,
			GST_MAKE_FOURCC('N', 'V', '1', '2'), "width", G_TYPE_INT, 640,
			"height", G_TYPE_INT, 480, NULL );

	link_ok = gst_element_link_filtered(v4l2src, dmaiaccel, caps);
	gst_caps_unref(caps);

	if (!link_ok) {
		g_warning("Failed to link v4l2src and dmaiaccel\n");
	}

	return link_ok;
}

int streaming_start(int argc, char *argv[], char *host, int port) {
	GstElement *pipeline, *source, *dmaiaccel, *queue, *encoder, *sink;
	int error = 0;

	if (loop) {
		printf("INFO: Stopping previous stream\n");
		streaming_stop();
	}

	printf("INFO: Initializing stream for client@%s:%d\n", host, port);
	gst_init(&argc, &argv);

	printf("INFO: Creating streaming loop\n");
	loop = g_main_loop_new(NULL, FALSE);

	pipeline = gst_pipeline_new("video-streamer");
	source = gst_element_factory_make("v4l2src", "source");
	dmaiaccel = gst_element_factory_make("dmaiaccel", NULL );
	queue = gst_element_factory_make("queue", NULL );
	encoder = gst_element_factory_make("dmaienc_mpeg4", "encoder");
	sink = gst_element_factory_make("udpsink", "sink");

	if (!source) {
		g_printerr("Failed to create the v4l2src element\n");
		error = -1;
	}

	if (!dmaiaccel || !encoder) {
		g_printerr("Failed to create the DMAI element(s)\n");
		error = -1;
	}

	if (!queue) {
		g_printerr("Failed to create the queue element\n");
		error = -1;
	}

	if (!sink) {
		g_printerr("Failed to create the udpsink element\n");
		error = -1;
	}

	if (error < 0) {
		return error;
	}

	g_object_set(source, "always_copy", FALSE, NULL );
	g_object_set(sink, "host", host, "port", port, NULL );

	gst_bin_add_many(GST_BIN(pipeline), source, dmaiaccel, queue, encoder, sink,
			NULL );

	media_link_elements_with_filter(source, dmaiaccel);
	gst_element_link(dmaiaccel, queue);
	gst_element_link(queue, encoder);
	gst_element_link(encoder, sink);

	gst_element_set_state(pipeline, GST_STATE_PLAYING);

	printf("INFO: Streaming started\n");
	g_main_loop_run(loop);

	printf("INFO: Cleaning up stream resources\n");
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline) );

	return 0;
}

void streaming_stop() {
	if (!loop)
		return;

	printf("INFO: Stopping streaming loop\n");
	g_main_loop_quit(loop);
}
