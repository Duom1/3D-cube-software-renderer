#include <math.h>
#include <raylib.h>
#include <stdio.h>

typedef struct {
  double x;
  double y;
} Vec2;

typedef struct {
  double x;
  double y;
  double z;
} Vec3;

typedef struct {
  double x;
  double y;
  double z;
  double w;
} Vec4;

typedef double Matrix4x4[4][4];

void M4x4TimesM4x4(Matrix4x4 a, Matrix4x4 b) {
  Matrix4x4 ans = {0};
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      ans[i][j] = 0;
      for (int k = 0; k < 4; ++k) {
        ans[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      a[i][j] = ans[i][j];
    }
  }
}

Vec4 M4x4TimesVec4(Matrix4x4 m, Vec4 v) {
  Vec4 result;
  result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
  result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
  result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;
  result.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;
  return result;
}

Vec2 normVec4ToVec2(Vec4 v) {
  Vec2 ans;
  ans.x = v.x / v.w;
  ans.y = v.y / v.w;
  return ans;
}

Vec2 normVec2ToPx(Vec2 v, Vec2 size) {
  return (Vec2){size.x / 2 * (v.x + 1.0), size.y / 2 * (1.0 + v.y)};
}

Vec2 projToPx(Vec3 point, Matrix4x4 proj, Vec2 size) {
  return (Vec2)normVec2ToPx((Vec2)normVec4ToVec2((Vec4)M4x4TimesVec4(
                                proj, (Vec4){point.x, point.y, point.z, 1})),
                            size);
}

void printM4x4(FILE *f, Matrix4x4 a) {
  for (int i = 0; i < 4; ++i) {
    fprintf(f, "%f, %f, %f, %f\n", a[i][0], a[i][1], a[i][2], a[i][3]);
  }
}

void printVec4(FILE *f, Vec4 a) {
  fprintf(f, "%f, %f, %f, %f\n", a.x, a.y, a.z, a.w);
}

void printVec3(FILE *f, Vec3 a) { fprintf(f, "%f, %f, %f\n", a.x, a.y, a.z); }

void printVec2(FILE *f, Vec2 a) { fprintf(f, "%f, %f\n", a.x, a.y); }

Vec3 dropW(Vec4 v) { return (Vec3){v.x, v.y, v.z}; }

double edgeFunction(Vec2 a, Vec2 b, Vec2 c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

int main(void) {
  static const double fov = 90 * M_PI / 360;
  static const double near = 1.0;
  static const double far = 90.0;
  Vec2 size = {800, 800};
  double ar = size.y / size.x;
  static bool wireFrame = false;
  const double tanFov = tan(fov);
  Matrix4x4 proj = {
      {1 / tanFov * ar, 0, 0, 0},
      {0, 1 / tanFov, 0, 0},
      {0, 0, (far + near) / (near - far), 2 * far * near / (near - far)},
      {0, 0, -1, 0},
  };
  static Matrix4x4 movez = {
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 1.5},
      {0, 0, 0, 1},
  };

  double angleB = .03;
  double angleC = .015;
  Matrix4x4 rotate2 = {
      {1, 0, 0, 0},
      {0, cos(angleB), sin(angleB), 0},
      {0, -sin(angleB), cos(angleB), 0},
      {0, 0, 0, 1},
  };
  Matrix4x4 rotate = {
      {cos(angleC), -sin(angleC), 0, 0},
      {sin(angleC), cos(angleC), 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1},
  };
  M4x4TimesM4x4(rotate, rotate2);

#define VS 9
#define TS 12
  // clang-format off
  static Vec3 points[VS] = {
      {-.5, -.5, -.5}, 
      {-.5, .5, -.5}, 
      {.5, .5, -.5}, 
      {.5, -.5, -.5},  
      {-.5, -.5, .5},
      {-.5, .5, .5},
      {.5, .5, .5},
      {.5, -.5, .5},
      {0, 0, 0}
  };
  static int triangles[TS][3] = {
    {0, 1, 3},
    {1, 2, 3},
    {1, 5, 2},
    {5, 6, 2},
    {3, 2, 7},
    {2, 6, 7},
    {6, 4, 7},
    {6, 5, 4},
    {0, 7, 4},
    {0, 3, 7},
    {5, 0, 4},
    {5, 1, 0},
  };
  // clang-format on
  static Vec2 pointsPx[VS - 1];

  for (int i = 0; i < VS; ++i) {
    Vec4 tmp = (Vec4){points[i].x, points[i].y, points[i].z, 1};
    points[i] = dropW(M4x4TimesVec4(movez, tmp));
  }

  InitWindow(size.x, size.y, "window");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    if (IsWindowResized()) {
      size = (Vec2){GetScreenWidth(), GetScreenHeight()};
      ar = size.y / size.x;
      proj[0][0] = 1 / tanFov * ar;
      proj[1][1] = 1 / tanFov;
      proj[2][2] = (far + near) / (near - far);
      proj[2][3] = 2 * far * near / (near - far);
      proj[3][2] = -1;
    }
    if (IsKeyPressed(KEY_W))
      wireFrame = !wireFrame;
    for (int i = 0; i < VS - 1; ++i) {
      Vec4 tmp =
          (Vec4){points[i].x - points[VS - 1].x, points[i].y - points[VS - 1].y,
                 points[i].z - points[VS - 1].z, 1};
      points[i] = dropW(M4x4TimesVec4(rotate, tmp));
      points[i] =
          (Vec3){points[i].x + points[VS - 1].x, points[i].y + points[VS - 1].y,
                 points[i].z + points[VS - 1].z};
      pointsPx[i] = projToPx(points[i], proj, size);
    }
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (int i = 0; i < TS; ++i) {
      Color color = BLACK;
      if (i < 2)
        color = BLUE;
      else if (i < 4 && i >= 2)
        color = RED;
      else if (i < 6 && i >= 4)
        color = GREEN;
      else if (i < 8 && i >= 6)
        color = BLUE;
      else if (i < 10 && i >= 8)
        color = RED;
      else if (i < 12 && i >= 10)
        color = GREEN;
      if (wireFrame) {
        DrawTriangleLines(
            (Vector2){pointsPx[triangles[i][0]].x, pointsPx[triangles[i][0]].y},
            (Vector2){pointsPx[triangles[i][1]].x, pointsPx[triangles[i][1]].y},
            (Vector2){pointsPx[triangles[i][2]].x, pointsPx[triangles[i][2]].y},
            color);
      } else {
        DrawTriangle(
            (Vector2){pointsPx[triangles[i][0]].x, pointsPx[triangles[i][0]].y},
            (Vector2){pointsPx[triangles[i][1]].x, pointsPx[triangles[i][1]].y},
            (Vector2){pointsPx[triangles[i][2]].x, pointsPx[triangles[i][2]].y},
            color);
      }
    }
    DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
