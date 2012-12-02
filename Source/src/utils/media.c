#include "media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 #include <gst/gst.h>
 #include <glib.h>

 gboolean media_link_elements_with_filter(GstElement *v4l2src,
 GstElement *dmaiaccel) {
 gboolean link_ok;
 GstCaps *caps;

 caps = gst_caps_new_simple("video/x-raw-yuv", "format", G_TYPE_STRING,
 "NV12", "width", "width", G_TYPE_INT, 384, "height", G_TYPE_INT,
 288, NULL );

 link_ok = gst_element_link_filtered(v4l2src, dmaiaccel, caps);
 gst_caps_unref(caps);

 if (!link_ok) {
 g_warning("Failed to link v4l2src and dmaiaccel.");
 }

 return link_ok;
 }

void media_stream_start(int argc, char *argv[], char *host, int port) {
 GMainLoop *loop;
 GstElement *pipeline, *source, *dmaiaccel, *queue, *encoder, *sink;
 GstCaps *caps;
 int error = 0;

 gst_init(&argc, &argv);
 loop = g_main_loop_new(NULL, FALSE);

 pipeline = gst_pipeline_new("video-streamer");
 source = gst_element_factory_make("v4l2src", "source");
 dmaiaccel = gst_element_factory_make("dmaiaccel", NULL );
 queue = gst_element_factory_make("queue", NULL );
 encoder = gst_element_factory_make("dmaienc_mpeg4", "encoder");
 sink = gst_element_factory_make("udpsink", "sink");

 if (!source) {
 g_printerr("Failed to create the v4l2src element.");
 error = -1;
 }

 if (!dmaiaccel || !encoder) {
 g_printerr("Failed to create the DMAI element(s).");
 error = -1;
 }

 if (!queue) {
 g_printerr("Failed to create the queue element");
 error = -1;
 }

 if (!sink) {
 g_printerr("Failed to create the udpsink element");
 error = -1;
 }

 if (error < 0) {
 return error;
 }

 g_object_set(source, "always_copy", FALSE, NULL );
 g_object_set(source, "host", host, "port", port, NULL );

 gst_bin_add_many(GST_BIN(pipeline), source, dmaiaccel, queue, encoder, sink,
 NULL );
 gst_element_link_many(source, dmaiaccel, queue, encoder, sink, NULL );
 media_link_elements_with_filter(source, dmaiaccel);
 gst_element_set_state(pipeline, GST_STATE_PLAYING);
 }

 */
