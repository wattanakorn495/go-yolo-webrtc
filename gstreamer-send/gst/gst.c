#include "gst.h"
#include "avi_yolo.h"
#include <gst/app/gstappsrc.h>
#include <pthread.h>
#include <unistd.h>

static pthread_t detect_thread;
static pthread_mutex_t lock;
static image image_buffer;

static int running=0;


float get_pixel(image m, int x, int y, int c)
{
  g_assert(x < m.w && y < m.h && c < m.c);
  return m.data[c*m.h*m.w + y*m.w + x];
}
void set_pixel(image m, int x, int y, int c, float val)
{
  g_assert(x < m.w && y < m.h && c < m.c);
  m.data[c*m.h*m.w + y*m.w + x] = val;
}

void guchar_to_image(guchar *pixels, image im) 
{
  int stride =((im.w * 3)+3)&~3;
  for(int j = 0; j < im.h; ++j){
    for(int i = 0; i < im.w; ++i){
      guchar *p = pixels + j * stride + i * 3;
      set_pixel(im, i, j, 0,(float)(p[0])/255.0);
      set_pixel(im, i, j, 1,(float)(p[1])/255.0);
      set_pixel(im, i, j, 2,(float)(p[2])/255.0);
    }
  }
}

void image_to_guchar(image im, guchar *pixels) 
{
  int stride =((im.w * 3)+3)&~3;
  for(int j = 0; j < im.h; ++j){
    for(int i = 0; i < im.w; ++i){
      guchar *p = pixels + j * stride + i * 3;
      p[0] =(unsigned char)(get_pixel(im, i, j, 0)*255.0);
      p[1] =(unsigned char)(get_pixel(im, i, j, 1)*255.0);
      p[2] =(unsigned char)(get_pixel(im, i, j, 2)*255.0);
    }
  }
}

static void *detect_image_thread(void *ptr)
{
  while(running)
  {
    double starttime = what_time_is_it_now();
    //g_print("In thread\n");
    pthread_mutex_lock(&lock);
    //g_print("In lock \n");
    /* darknet */
    predict_image(image_buffer);
    
    //g_print("nboxes: %d, classes: %d\n", nboxes, l.classes);
    pthread_mutex_unlock(&lock);

    double timediff = what_time_is_it_now() - starttime;
    //double fps = 1.0/timediff;
    //printf("%.02f sec, %.02f fps\n", timediff, fps);
    usleep(1000);
  }
  return NULL;
}

typedef struct SampleHandlerUserData {
  int pipelineId;
} SampleHandlerUserData;

GMainLoop *gstreamer_send_main_loop = NULL;
void gstreamer_send_start_mainloop(void) {
  gstreamer_send_main_loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(gstreamer_send_main_loop);
}

static gboolean gstreamer_send_bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
  switch (GST_MESSAGE_TYPE(msg)) {

    case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    exit(1);
    break;

    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *error;

      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);

      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      exit(1);
      break;
    }
    default:
    break;
  }

  return TRUE;
}

GstFlowReturn gstreamer_send_new_sample_handler(GstElement *object, gpointer user_data) {
  GstSample *sample = NULL;
  GstBuffer *buffer = NULL;
  gpointer copy = NULL;
  gsize copy_size = 0;
  SampleHandlerUserData *s = (SampleHandlerUserData *)user_data;

  g_signal_emit_by_name (object, "pull-sample", &sample);
  if (sample) {
    buffer = gst_sample_get_buffer(sample);
    if (buffer) {
      gst_buffer_extract_dup(buffer, 0, gst_buffer_get_size(buffer), &copy, &copy_size);
      goHandlePipelineBuffer(copy, copy_size, GST_BUFFER_DURATION(buffer), s->pipelineId);
    }
    gst_sample_unref (sample);
  }

  return GST_FLOW_OK;
}

static GstPadProbeReturn
cb_have_data (GstPad          *pad,
  GstPadProbeInfo *info,
  gpointer         user_data)
{
 // gint x, y;
  GstMapInfo map;
  //RGB* ptr, t;
  GstBuffer *buffer;

  buffer = GST_PAD_PROBE_INFO_BUFFER (info);

  buffer = gst_buffer_make_writable (buffer);
  //g_print("%d\n",(int)gst_buffer_get_size (buffer));

  /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
  if (buffer == NULL)
    return GST_PAD_PROBE_OK;

  /* Mapping a buffer can fail (non-writable) */
  if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {    
    /* invert data */
    pthread_mutex_lock(&lock);
    //g_print("In callback \n");
    guchar_to_image(map.data, image_buffer);
    pthread_mutex_unlock(&lock);
    
    
    draw(image_buffer);

    pthread_mutex_lock(&lock);
    image_to_guchar(image_buffer, map.data);
    pthread_mutex_unlock(&lock);

    gst_buffer_unmap (buffer, &map);
  }

  GST_PAD_PROBE_INFO_DATA (info) = buffer;

  return GST_PAD_PROBE_OK;
}

GstElement *gstreamer_send_create_pipeline(char *pipeline) {
  GstElement *pipeline_ob, *src, *vs, *filter, *vc, *filter1, *col,*vk, *enc, *sink;
  GstCaps *filtercaps, *filtercaps1;
  GstPad *pad;

  gst_init(NULL, NULL);
  GError *error = NULL;
  char *data_cfg = "cfg/coco.data";
  char *cfg = "cfg/yolov3-tiny.cfg";
  char *weights = "yolov3-tiny.weights";
  init_yolo(data_cfg, cfg, weights);

  int width=  320*3;
  int height= 180*3;
  image_buffer = make_image(width, height, 3);
  /* thread */
  running=1;
  if (running)
  {   
    if (pthread_mutex_init(&lock, NULL) != 0) {
      g_print("mutex init failed\n");
    } else{
      g_print("mutex init OK\n");
    }
    if (pthread_create(&detect_thread, NULL, detect_image_thread, NULL)) {
      g_print("Thread creation failed\n");
    } else{
      g_print("Thread creation OK\n");
    }
  }
  g_print("Yoyo\n");
  if (strcmp(pipeline,"test") == 0)
  {
   g_print("gst test\n");
    /* build */
    pipeline_ob = gst_pipeline_new ("my-pipeline");
    //pipeline_ob = gst_parse_launch(" videotestsrc ! vp8enc ! appsink name=appsink ", &error);
    
    src = gst_element_factory_make ("v4l2src", "src");
    if (src == NULL)
      g_error ("Could not create 'v4l2src' element");

    vs = gst_element_factory_make ("videoscale", "vs");
    if (vs == NULL)
      g_error ("Could not create 'videoscale' element");

    filter = gst_element_factory_make ("capsfilter", "filter");
    g_assert (filter != NULL); /* should always exist */

    vc = gst_element_factory_make ("videoconvert", "vc");
    if (vc == NULL)
      g_error ("Could not create 'videoconvert' element");

    filter1 = gst_element_factory_make ("capsfilter", "filter1");
    g_assert (filter1 != NULL); /* should always exist */

    col = gst_element_factory_make ("clockoverlay", "col");
    if (col == NULL)
      g_error ("Could not create 'clockoverlay' element");

    vk = gst_element_factory_make ("videoconvert", "vk");
    if (vk == NULL)
      g_error ("Could not create 'videoconvert' element");

    enc = gst_element_factory_make ("vp8enc", "enc");
    if (enc== NULL)
      g_error ("Could not create 'vp8enc' element");

    sink = gst_element_factory_make ("appsink", "appsink");
    if (sink== NULL)
      g_error ("Could not create 'appsink' element");

    gst_bin_add_many (GST_BIN (pipeline_ob), src, vs, filter, vc, filter1, col, vk, enc, sink, NULL);
    gst_element_link_many (src, vs, filter, vc, filter1, col, vk, enc, sink, NULL);
    
    //g_object_set (G_OBJECT (src), "io-mode", 3, NULL);
    g_object_set (G_OBJECT (enc), "threads", 64, NULL);

    
    int width= 320*3;
    int height= 180*3;

    filtercaps = gst_caps_new_simple ("video/x-raw",
               //"format", G_TYPE_STRING, "RGB",
               "width", G_TYPE_INT, width,
               "height", G_TYPE_INT, height,
               //"framerate", GST_TYPE_FRACTION, 15, 1,
               NULL);
    g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
    gst_caps_unref (filtercaps);
    
    filtercaps1 = gst_caps_new_simple ("video/x-raw",
               // "framerate", GST_TYPE_FRACTION, 30, 1,
               "format", G_TYPE_STRING, "RGB",
               //"width", G_TYPE_INT, 384,
               //"height", G_TYPE_INT, 288,
               //"framerate", GST_TYPE_FRACTION, 30, 1,
               NULL);
    g_object_set (G_OBJECT (filter1), "caps", filtercaps1, NULL);
    gst_caps_unref (filtercaps1);
    
    g_object_set (G_OBJECT (enc), "deadline", 1, NULL);
    g_object_set (G_OBJECT (enc), "cpu-used", 16, NULL);

    g_object_set (G_OBJECT (col), "halignment", 1, NULL);
    g_object_set (G_OBJECT (col), "valignment", 1, NULL);
    g_object_set (G_OBJECT (col), "text", "Avilon yolo webrtc", NULL);
    g_object_set (G_OBJECT (col), "shaded-background", TRUE, NULL);
    g_object_set (G_OBJECT (col), "font-desc", "Sans, 36", NULL);
    
    pad = gst_element_get_static_pad (vc, "src");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_data, NULL, NULL);
    gst_object_unref (pad);
    
    return pipeline_ob ;
  }
  else{
    return gst_parse_launch(" videotestsrc ! vp8enc ! appsink name=appsink ", &error);
  }
}

void gstreamer_send_start_pipeline(GstElement *pipeline, int pipelineId) {
  SampleHandlerUserData *s = calloc(1, sizeof(SampleHandlerUserData));
  s->pipelineId = pipelineId;



  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  guint bus_watch_id = gst_bus_add_watch(bus, gstreamer_send_bus_call, NULL);
  gst_object_unref(bus);

  GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
  g_object_set(appsink, "emit-signals", TRUE, NULL);
  g_signal_connect(appsink, "new-sample", G_CALLBACK(gstreamer_send_new_sample_handler), s);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void gstreamer_send_stop_pipeline(GstElement *pipeline) {
  gst_element_set_state(pipeline, GST_STATE_NULL);
}


