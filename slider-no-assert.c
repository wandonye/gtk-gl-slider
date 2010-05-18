/* un deslizador de planos OpenGL texturados con Cairo */
/* una implementación mínima sin aserciones para dispositivos embebidos */
/* Juan Manuel Mouriz <jmouriz@gmail.com> */

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include <math.h>

#define IMAGE "covers/1.png"
#define SPEED 1000 / 60

typedef enum
{
  BACK = -1,
  FORWARD = 1,
  LEFT = -1,
  RIGHT = 1
} direction_t;

static GtkWidget *drawing_area;
static gfloat angle;
static gfloat offset;
static GLuint texture;
static direction_t direction;

gint width;
gint height;
guchar *surface_data[1];

/* función de apoyo que reemplaza gluPerspective con glFrustum */
void
perspective (GLdouble fovy, GLdouble aspect, GLdouble near, GLdouble far)
{
  GLdouble top;
  GLdouble bottom;
  GLdouble left;
  GLdouble right;

  top = near * tan (M_PI / 180 * fovy / 2);
  bottom = -top;
  right = aspect * top;
  left = -right;

  glFrustum (left, right, bottom, top, near, far);
}

/* transformación para la animación de rotación */
void
transform_rotation (void)
{
  glRotatef (angle * direction, 0, 1, 0);
}

/* transformación para la animación de desplazamiento */
void
transform_translation (void)
{
  glTranslatef (offset * direction, 0.0f, 0.0f);
}

/* actualización del área de dibujo */
void
update (void)
{
  gdk_window_invalidate_rect (drawing_area->window, &drawing_area->allocation, FALSE);
  gdk_window_process_updates (drawing_area->window, FALSE);
}

/* función de rotación */
gboolean
rotate (gpointer data)
{
  gboolean stop;

  stop = FALSE;
  angle += 10.0f;
  stop ^= angle > 45.0f;

  update ();

  return !stop;
}

/* función de desplazamiento */
gboolean
translate (gpointer data)
{
  gboolean stop;

  stop = FALSE;
  offset += 0.05f;
  stop ^= offset > 0.5f;

  update ();

  return !stop;
}

/* comienzo de la rotación */
void
start_rotation (void)
{
  angle = 0.0f;
  g_timeout_add (SPEED, rotate, NULL);
}

/* comienzo del desplazamiento */
void
start_translation (void)
{
  offset = 0.0f;
  g_timeout_add (SPEED, translate, NULL);
}

/* manejadores de señales para los botones de control */

void
go_left (GtkWidget *widget, gpointer data)
{
  direction = LEFT;
  start_rotation ();
}

void
go_right (GtkWidget *widget, gpointer data)
{
  direction = RIGHT;
  start_rotation ();
}

void
go_back (GtkWidget *widget, gpointer data)
{
  direction = BACK;
  start_translation ();
}

void
go_forward (GtkWidget *widget, gpointer data)
{
  direction = FORWARD;
  start_translation ();
}

void
go_center (GtkWidget *widget, gpointer data)
{
  angle = 0.0f;
  offset = 0.0f;

  update ();
}

/* dibujar un plano */
void
draw_cover (float offset, float angle, float near, gboolean animate)
{
    glPushMatrix ();
    glTranslatef (offset, 0.0f, near);
    glRotatef (angle, 0, 1, 0);
    if (animate)
      transform_rotation ();
    glBegin (GL_QUADS);
    glTexCoord2i (width, height);
    glVertex2f (0.5f, -0.5f);
    glTexCoord2i (width, -0.5f);
    glVertex2f (0.5f, 0.5f);
    glTexCoord2i (-0.5f, -0.5f);
    glVertex2f (-0.5f, 0.5f);
    glTexCoord2i (-0.5f, height);
    glVertex2f (-0.5f, -0.5f);
    glEnd ();
    glPopMatrix ();
}

/* dibujar una pila de planos */
void
draw_covers_stack (void)
{
  float x;
  int i;

  glPushMatrix ();
  transform_translation ();
  for (x = 1.0f; x > 0.2f; x -= 0.1f)
    draw_cover (x, -45.0f, 0.0f, FALSE);
  for (x = -1.0f; x < -0.2f; x += 0.1f)
    draw_cover (x, 45.0f, 0.0f, FALSE);
  draw_cover (0.0f, 0.0f, 0.25f, TRUE);
  glPopMatrix ();
}

/* barra de control de desplazamiento */
GtkWidget *
build_translation_control (void)
{
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *label;

  box = gtk_hbox_new (TRUE, 0);

  label = gtk_label_new ("<b>Desplazamiento</b>");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_container_add (GTK_CONTAINER (box), label);
  gtk_widget_show (label);

  button = gtk_button_new_with_label ("Izquierda");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_back), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Centro");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_center), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Derecha");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_forward), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  return box;
}

/* barra de control de rotación */
GtkWidget *
build_rotation_control (void)
{
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *label;

  box = gtk_hbox_new (TRUE, 0);

  label = gtk_label_new ("<b>Rotación</b>");

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_container_add (GTK_CONTAINER (box), label);
  gtk_widget_show (label);

  button = gtk_button_new_with_label ("Izquierda");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_left), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Centro");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_center), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("Derecha");

  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (go_right), NULL);

  gtk_container_add (GTK_CONTAINER (box), button);
  gtk_widget_show (button);

  return box;
}

/* barra de control */
GtkWidget *
build_control (void)
{
  GtkWidget *box;
  GtkWidget *control;

  box = gtk_vbox_new (FALSE, 0);

  control = build_rotation_control ();

  gtk_container_add (GTK_CONTAINER (box), control);
  gtk_widget_show (control);

  control = build_translation_control ();

  gtk_container_add (GTK_CONTAINER (box), control);
  gtk_widget_show (control);

  return box;
}

/* manejadores de señales */

gboolean
delete (GtkWidget *widget, GdkEvent *event, gpointer data)
{
  glDeleteTextures (1, &texture);
  gtk_main_quit ();
  return TRUE;
}

void
realize (GtkWidget *widget, gpointer data)
{
  GLfloat light[4];
  GdkGLContext *gl_context;
  GdkGLDrawable *gl_drawable;

  gl_context = gtk_widget_get_gl_context (widget);
  gl_drawable = gtk_widget_get_gl_drawable (widget);

  light[0] = 1.0f;
  light[1] = 1.0f;
  light[2] = 1.0f;
  light[3] = 1.0f;

  gdk_gl_drawable_gl_begin (gl_drawable, gl_context);

  glEnable (GL_TEXTURE_RECTANGLE_ARB);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glLightfv (GL_LIGHT0, GL_DIFFUSE, light);

  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  gdk_gl_drawable_gl_end (gl_drawable);
}

gboolean
configure (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  GdkGLContext *gl_context;
  GdkGLDrawable *gl_drawable;

  gl_context = gtk_widget_get_gl_context (widget);
  gl_drawable = gtk_widget_get_gl_drawable (widget);
  width = widget->allocation.width;
  height = widget->allocation.height;

  gdk_gl_drawable_gl_begin (gl_drawable, gl_context);

  glViewport (0, 0, width, height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  perspective (60.0f, (GLfloat) width / (GLfloat) height, 1.0f, 30.0f);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (0.0f, 0.0f, -2.0f);
  glGenTextures (1, &texture);

  gdk_gl_drawable_gl_end (gl_drawable);

  return TRUE;
}

gboolean
expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  GdkGLContext *gl_context;
  GdkGLDrawable *gl_drawable;
  gboolean is_double_buffered;

  gl_context = gtk_widget_get_gl_context (widget);
  gl_drawable = gtk_widget_get_gl_drawable (widget);
  is_double_buffered = gdk_gl_drawable_is_double_buffered (gl_drawable);

  gdk_gl_drawable_gl_begin (gl_drawable, gl_context);

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
  glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface_data[0]);
  glPushMatrix ();
  draw_covers_stack ();
  glPopMatrix ();

  if (is_double_buffered)
    gdk_gl_drawable_swap_buffers (gl_drawable);
  else
    glFlush ();

  gdk_gl_drawable_gl_end (gl_drawable);

  return TRUE;
}

/* inicio */
int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *control;
  GdkGLConfig *gl_config;
  cairo_surface_t *surface;
  cairo_t *context;

  gtk_init (&argc, &argv);
  gtk_gl_init (&argc, &argv);

  gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_ALPHA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title (GTK_WINDOW (window), "Desplazamiento de planos animado");

  gtk_widget_add_events (window, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  g_signal_connect (G_OBJECT (window), "delete-event", G_CALLBACK (delete), NULL);

  box = gtk_vbox_new (FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show (box);

  label = gtk_label_new ("<span size='18000' weight='heavy'>Desplazamiento de planos animado</span>");

  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_container_add (GTK_CONTAINER (box), label);
  gtk_widget_show (label);

  drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (box), drawing_area);
  gtk_widget_show (drawing_area);

  control = build_control ();

  gtk_box_pack_start (GTK_BOX (box), control, FALSE, FALSE, 0);
  gtk_widget_show (control);

  /* cargar la imagen en un contexto Cairo y obtener los bytes para usar como textura de los planos */
  surface = cairo_image_surface_create_from_png (IMAGE);

  width = cairo_image_surface_get_width (surface);
  height = cairo_image_surface_get_height (surface);

  gtk_widget_set_size_request (drawing_area, width, height);
  gtk_widget_set_gl_capability (drawing_area, gl_config, NULL, TRUE, GDK_GL_RGBA_TYPE);

  g_signal_connect_after (G_OBJECT (drawing_area), "realize", G_CALLBACK (realize), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "configure-event", G_CALLBACK (configure), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "expose-event", G_CALLBACK (expose), NULL);

  gtk_widget_set_app_paintable (window, TRUE);
  gtk_widget_realize (window);
  gdk_window_set_back_pixmap (window->window, NULL, FALSE);
  gtk_widget_show (window);

  surface_data[0] = g_malloc0 (4 * width * height);

  context = cairo_create (surface);

  surface_data[0] = cairo_image_surface_get_data (surface);

  gtk_main ();

  return 0;
}
