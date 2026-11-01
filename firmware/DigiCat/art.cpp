#include "art.h"

namespace {

// Maps cat-space coordinates (origin between the paws, 1 unit = 1 px for an adult) to the target
struct Painter {
  lgfx::LovyanGFX& g;
  int cx, fy;
  float s;

  int X(float dx) const { return cx + int(lroundf(dx * s)); }
  int Y(float dy) const { return fy + int(lroundf(dy * s)); }
  int R(float r) const { return max(1, int(lroundf(r * s))); }

  void ellipse(float x, float y, float rx, float ry, uint16_t c) { g.fillEllipse(X(x), Y(y), R(rx), R(ry), c); }
  void circle(float x, float y, float r, uint16_t c) { g.fillCircle(X(x), Y(y), R(r), c); }
  void tri(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t c) {
    g.fillTriangle(X(x0), Y(y0), X(x1), Y(y1), X(x2), Y(y2), c);
  }
  void line(float x0, float y0, float x1, float y1, float w, uint16_t c) {
    g.drawWideLine(X(x0), Y(y0), X(x1), Y(y1), max(0.5f, w * s), c);
  }
  // Arc stroke; angles in degrees, 0 = right, clockwise
  void arc(float x, float y, float r, float w, float a0, float a1, uint16_t c) {
    int rr = R(r);
    g.fillArc(X(x), Y(y), rr, max(0, rr - max(1, int(w * s + 0.5f))), a0, a1, c);
  }
};

void drawEye(Painter& p, float ex, float ey, const CatPose& pose) {
  using E = CatPose::Eyes;
  switch (pose.eyes) {
    case E::Open:
    case E::Sad: {
      float lx = pose.look / max(0.5f, p.s);
      float ry = pose.eyes == E::Sad ? 7 : 9;
      p.ellipse(ex + lx, ey, 7, ry, pal::eye);
      p.ellipse(ex + lx, ey + 1, 4, ry - 2, pal::ink);
      p.circle(ex + lx - 2, ey - 3, 2, pal::white);
      if (pose.eyes == E::Sad) {
        float dir = ex < 0 ? 1 : -1;  // brows slope down towards the outside
        p.line(ex - 7 * dir, ey - 10, ex + 6 * dir, ey - 13, 1.5f, pal::ink);
      }
      break;
    }
    case E::Closed:
      p.arc(ex, ey - 2, 7, 2.2f, 20, 160, pal::ink);   // ︶
      break;
    case E::Happy:
      p.arc(ex, ey + 4, 7, 2.2f, 200, 340, pal::ink);  // ^
      break;
  }
}

void drawMouth(Painter& p, float y, CatPose::Mouth m) {
  using M = CatPose::Mouth;
  switch (m) {
    case M::Smile:
      p.arc(-3.5f, y, 3.5f, 1.6f, 0, 180, pal::ink);
      p.arc(3.5f, y, 3.5f, 1.6f, 0, 180, pal::ink);
      break;
    case M::Neutral:
      p.line(-3, y + 2, 3, y + 2, 1.2f, pal::ink);
      break;
    case M::Frown:
      p.arc(0, y + 6, 4, 1.6f, 200, 340, pal::ink);
      break;
    case M::Open:
      p.ellipse(0, y + 3, 4, 4, pal::ink);
      p.ellipse(0, y + 5, 2.5f, 2, pal::pink);
      break;
  }
}

void drawWhiskers(Painter& p, float y) {
  for (int side = -1; side <= 1; side += 2) {
    p.line(side * 18, y - 3, side * 42, y - 7, 0.8f, pal::ink);
    p.line(side * 18, y, side * 43, y + 1, 0.8f, pal::ink);
    p.line(side * 18, y + 3, side * 40, y + 9, 0.8f, pal::ink);
  }
}

void drawEars(Painter& p, float headY, uint16_t fur, uint16_t stripe) {
  for (int side = -1; side <= 1; side += 2) {
    p.tri(side * 31, headY - 6, side * 27, headY - 38, side * 7, headY - 24, fur);
    p.tri(side * 25, headY - 11, side * 24, headY - 31, side * 12, headY - 22, pal::pink);
    p.line(side * 27, headY - 38, side * 25, headY - 34, 1.5f, stripe);
  }
}

}  // namespace

void drawCatSitting(lgfx::LovyanGFX& g, int cx, int floorY, const CatPose& pose) {
  Painter p{g, cx, floorY, pose.scale};
  const float b = pose.breathe;

  // Tail (behind the body): quadratic curve sweeping up, tip swaying
  {
    float x0 = 20, y0 = -8, x1 = 54, y1 = -12, x2 = 44 + pose.tail * 12, y2 = -58;
    const int n = 16;
    for (int i = 0; i <= n; i++) {
      float t = float(i) / n, u = 1 - t;
      float x = u * u * x0 + 2 * u * t * x1 + t * t * x2;
      float y = u * u * y0 + 2 * u * t * y1 + t * t * y2;
      p.circle(x, y, 6.0f - 2.0f * t, i > n - 4 ? pose.stripe : pose.fur);
    }
  }

  // Body
  p.ellipse(0, -24 + b * 0.5f, 28, 24 + b * 0.5f, pose.fur);
  p.ellipse(0, -20, 16, 15, pal::belly);
  for (int side = -1; side <= 1; side += 2) {
    for (int i = 0; i < 3; i++) {
      float y = -36 + i * 9;
      p.line(side * 27, y, side * 19, y + 2, 1.8f, pose.stripe);
    }
  }

  // Front paws
  for (int side = -1; side <= 1; side += 2) {
    p.ellipse(side * 11, -4, 8, 5, pose.fur);
    p.line(side * 11, -6, side * 11, -1, 0.8f, pose.stripe);
  }

  // Head
  const float hy = -60 + b * 0.3f;
  drawEars(p, hy, pose.fur, pose.stripe);
  p.ellipse(0, hy, 33, 27, pose.fur);
  for (int side = -1; side <= 1; side += 2) {  // cheek fluff
    p.tri(side * 30, hy + 2, side * 39, hy + 9, side * 28, hy + 13, pose.fur);
  }
  p.line(-8, hy - 26, -6, hy - 18, 2, pose.stripe);  // forehead "M"
  p.line(0, hy - 27, 0, hy - 19, 2, pose.stripe);
  p.line(8, hy - 26, 6, hy - 18, 2, pose.stripe);

  drawEye(p, -13, hy - 2, pose);
  drawEye(p, 13, hy - 2, pose);

  if (pose.blush) {
    p.ellipse(-22, hy + 8, 6, 3, pal::blush);
    p.ellipse(22, hy + 8, 6, 3, pal::blush);
  }

  p.tri(-4, hy + 5, 4, hy + 5, 0, hy + 9, pal::pink);
  drawMouth(p, hy + 9, pose.mouth);
  drawWhiskers(p, hy + 8);
}

void drawCatSleeping(lgfx::LovyanGFX& g, int cx, int floorY, const CatPose& pose) {
  Painter p{g, cx, floorY, pose.scale};
  const float b = pose.breathe;

  // Curled body with stripes along the back
  p.ellipse(6, -18 - b * 0.5f, 42, 18 + b * 0.5f, pose.fur);
  for (int i = 0; i < 4; i++) {
    float x = -6 + i * 12;
    p.line(x, -35 - b * 0.5f, x + 3, -27, 2, pose.stripe);
  }

  // Tail wrapped around the front
  for (int i = 0; i <= 14; i++) {
    float a = 0.15f + i * 0.19f;
    float x = 6 + 44 * cosf(a), y = -8 + 7 * sinf(a);
    p.circle(x, y, 5.5f, i < 3 ? pose.stripe : pose.fur);
  }

  // Head resting on the left
  const float hx = -28, hy = -24;
  for (int side = -1; side <= 1; side += 2) {
    float ex = hx + side * 12;
    p.tri(ex - 9, hy - 10, ex + side * 2, hy - 30, ex + 9, hy - 12, pose.fur);
    p.tri(ex - 5, hy - 12, ex + side * 2, hy - 24, ex + 5, hy - 13, pal::pink);
  }
  p.ellipse(hx, hy, 24, 18, pose.fur);
  p.arc(hx - 9, hy - 1, 5, 2, 20, 160, pal::ink);
  p.arc(hx + 9, hy - 1, 5, 2, 20, 160, pal::ink);
  p.tri(hx - 3, hy + 5, hx + 3, hy + 5, hx, hy + 8, pal::pink);
  if (pose.blush) {
    p.ellipse(hx - 16, hy + 7, 5, 2.5f, pal::blush);
    p.ellipse(hx + 16, hy + 7, 5, 2.5f, pal::blush);
  }
}

void drawHeart(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c) {
  int h = max(1, r / 2);
  g.fillCircle(x - h, y, h + 1, c);
  g.fillCircle(x + h, y, h + 1, c);
  g.fillTriangle(x - r - 1, y + 1, x + r + 1, y + 1, x, y + r + h, c);
}

void drawFish(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c) {
  g.fillEllipse(x - r / 4, y, r * 3 / 4, r / 2, c);
  g.fillTriangle(x + r / 3, y, x + r, y - r / 2, x + r, y + r / 2, c);
  g.fillCircle(x - r / 2, y - 1, max(1, r / 8), pal::white);
}

void drawBall(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c) {
  g.fillCircle(x, y, r, c);
  g.fillArc(x, y, r, 0, 290, 340, pal::white);  // beach-ball wedges
  g.fillArc(x, y, r, 0, 110, 160, pal::white);
}

void drawBolt(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c) {
  g.fillTriangle(x + r / 3, y - r, x - r / 2, y + r / 6, x + r / 8, y + r / 6, c);
  g.fillTriangle(x - r / 3, y + r, x + r / 2, y - r / 6, x - r / 8, y - r / 6, c);
}

void drawMoon(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c, uint16_t bg) {
  g.fillCircle(x, y, r, c);
  g.fillCircle(x + r / 2, y - r / 3, r - 1, bg);
}

void drawBowl(lgfx::LovyanGFX& g, int x, int floorY, int fill) {
  const uint16_t bowl = rgb(70, 130, 220), rim = rgb(120, 175, 245), kibble = rgb(170, 100, 50);
  for (int i = 0; i < fill * 2; i++) g.fillCircle(x - 9 + i * 18 / max(1, fill * 2 - 1), floorY - 12 - (i % 2) * 2, 3, kibble);
  g.fillEllipse(x, floorY - 6, 15, 7, bowl);
  g.fillRect(x - 15, floorY - 12, 31, 6, bowl);
  g.fillEllipse(x, floorY - 12, 15, 3, rim);
  if (fill > 0) g.fillEllipse(x, floorY - 13, 11, 2, kibble);
}
