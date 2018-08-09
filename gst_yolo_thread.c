#include <gst/gst.h>
#include "darknet.h"
#include <pthread.h>
#include <unistd.h>

static image image_buffer;
static network *net;
char **names;
image **alphabet;
detection *dets;
layer l;
int running=0;
int classes;

static float thresh = 0.5;
static float hier = 0.5;
static float nms = 0.4;
static double fps;

static int nboxes = 0;


static pthread_t detect_thread;
static pthread_mutex_t lock;

static void *detect_image_thread(void *ptr);

typedef struct RGB
{
  guint8 red;
  guint8 green;
  guint8 blue;
} RGB;

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
  g_print("%d\n",(int)gst_buffer_get_size (buffer));

  /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
  if (buffer == NULL)
    return GST_PAD_PROBE_OK;

  /* Mapping a buffer can fail (non-writable) */
  if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {    
    /* invert data */
    pthread_mutex_lock(&lock);
    g_print("In callback \n");
    guchar_to_image(map.data, image_buffer);
    pthread_mutex_unlock(&lock);
    
    
    draw_detections(image_buffer, dets, nboxes, thresh, names, alphabet, l.classes);

    pthread_mutex_lock(&lock);
    image_to_guchar(image_buffer, map.data);
    pthread_mutex_unlock(&lock);

    gst_buffer_unmap (buffer, &map);
  }

  GST_PAD_PROBE_INFO_DATA (info) = buffer;

  return GST_PAD_PROBE_OK;
}

static void *detect_image_thread(void *ptr)
{
  while(running)
  {
    double starttime = what_time_is_it_now();
    //g_print("In thread\n");
    //pthread_mutex_lock(&lock);
    g_print("In lock \n");
    /* darknet */
    network_predict_image(net, image_buffer);
    dets = get_network_boxes(net, 1280, 720, thresh, hier, 0, 1, &nboxes);

    l = net->layers[net->n-1];
    if(nms > 0)
    { 
      do_nms_obj(dets, nboxes, l.classes, nms);
    }
    g_print("nboxes: %d, classes: %d\n", nboxes, l.classes);
    //pthread_mutex_unlock(&lock);

    double timediff = what_time_is_it_now() - starttime;
    double fps = 1.0/timediff;
    printf("%.02f sec, %.02f fps\n", timediff, fps);
    usleep(100);
  }
  return NULL;
}

void init_yolo(char *cfgfile, char *weightfile, char *namefile)
{

}

gint
main (gint   argc,
      gchar *argv[])
{
  GMainLoop *loop;
  GstElement *pipeline, *src, *sink, *filter, *csp, *csp1, *csp2, *filter1;
  GstCaps *filtercaps, *filtercaps1;
  GstPad *pad;


  /* init yolo */
  list *options = read_data_cfg("cfg/coco.data");
  char *name_list = option_find_str(options, "names", "data/names.list");
  names = get_labels(name_list);
  alphabet = load_alphabet();

  char *cfg = "cfg/yolov3-tiny.cfg";
  char *weights = "yolov3-tiny.weights";
  net = load_network(cfg,weights,0);

  layer l = net->layers[net->n-1];
  printf("classes: %d\n", l.classes );
  image_buffer = make_image(1280, 720, 3);

  /* thread */
  running=1;
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

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* build */
  pipeline = gst_pipeline_new ("my-pipeline");
  src = gst_element_factory_make ("v4l2src", "src");
  if (src == NULL)
    g_error ("Could not create 'videotestsrc' element");

  filter = gst_element_factory_make ("capsfilter", "filter");
  g_assert (filter != NULL); /* should always exist */

  csp = gst_element_factory_make ("videoconvert", "csp");
  if (csp == NULL)
    g_error ("Could not create 'videoconvert' element");

  csp1 = gst_element_factory_make ("videoconvert", "csp1");
  if (csp1 == NULL)
    g_error ("Could not create 'videoconvert' element");

  csp2 = gst_element_factory_make ("videorate", "csp2");
  if (csp2 == NULL)
    g_error ("Could not create 'videorate' element");

  filter1 = gst_element_factory_make ("capsfilter", "filter1");
  g_assert (filter1 != NULL); /* should always exist */

  sink = gst_element_factory_make ("xvimagesink", "sink");
  if (sink == NULL) {
    sink = gst_element_factory_make ("ximagesink", "sink");
    if (sink == NULL)
      g_error ("Could not create neither 'xvimagesink' nor 'ximagesink' element");
  }

  gst_bin_add_many (GST_BIN (pipeline), src, csp, filter, csp1, csp2, sink, NULL);
  gst_element_link_many (src, csp, filter, csp1, csp2, sink, NULL);
  
  filtercaps = gst_caps_new_simple ("video/x-raw",
               "format", G_TYPE_STRING, "RGB",
               //"width", G_TYPE_INT, 384,
               //"height", G_TYPE_INT, 288,
               //"framerate", GST_TYPE_FRACTION, 30, 1,
               NULL);
  g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);

  filtercaps1 = gst_caps_new_simple ("video/x-raw",
                "framerate", GST_TYPE_FRACTION, 30, 1,
               //"format", G_TYPE_STRING, "RGB",
               //"width", G_TYPE_INT, 384,
               //"height", G_TYPE_INT, 288,
               //"framerate", GST_TYPE_FRACTION, 30, 1,
               NULL);
  g_object_set (G_OBJECT (filter1), "caps", filtercaps1, NULL);
  gst_caps_unref (filtercaps1);
  
  
  pad = gst_element_get_static_pad (csp, "src");
  gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
      (GstPadProbeCallback) cb_have_data, NULL, NULL);
  gst_object_unref (pad);
  
  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
    g_error ("Failed to go into PLAYING state");
  }

  g_print ("Running ...\n");
  g_main_loop_run (loop);


  /* exit */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  (void) pthread_join(detect_thread, NULL);

  return 0;
}