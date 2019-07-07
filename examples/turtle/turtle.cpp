#include <cairo/cairo.h>

#include <cstdint>
#include <memory>
#include <string>

class Turtle final {
public:
  Turtle(int sizeX, int sizeY) {
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, sizeX, sizeY);
    context = cairo_create(surface);
    reset();
  }

  ~Turtle() {
    cairo_destroy(context);
    cairo_surface_destroy(surface);
  }

  void reset() {
    setColor(1, 1, 1);
    paint();
    setLineWidth(3);
    setColor(1, 0, 0);
    moveTo(0, 0);
  }

  void setColor(double r, double g, double b) { cairo_set_source_rgb(context, r, g, b); }

  void setLineWidth(double size) { cairo_set_line_width(context, size); }

  void paint() { cairo_paint(context); }

  void moveTo(double x, double y) { cairo_move_to(context, x, y); }

  void rectangleTo(double width, double height) {
    double baseX;
    double baseY;

    cairo_get_current_point(context, &baseX, &baseY);
    cairo_rectangle(context, baseX, baseY, width, height);
    cairo_fill(context);

    moveTo(baseX, baseY);
  }

  void lineTo(double x, double y) {
    cairo_line_to(context, x, y);
    cairo_stroke(context);
    moveTo(x, y);
  }

  void save(char const* filename) { cairo_surface_write_to_png(surface, filename); }

  void save(std::string const& filename) { save(filename.c_str()); }

private:
  cairo_surface_t* surface;
  cairo_t* context;
};

static std::unique_ptr<Turtle> turtle;

extern "C" {

void cofandina(int32_t x, int32_t y) {
  turtle.reset(new Turtle(x, y));
}

void pulitina(double r, double g, double b) {
  turtle->setColor(r, g, b);
}

void pastene(double size) {
  turtle->setLineWidth(size);
}

void muovi(double x, double y) {
  turtle->moveTo(x, y);
}

void ispettore(double x, double y) {
  turtle->lineTo(x, y);
}

void barilotto() {
  turtle->save("barilotto.png");
}

} // extern
