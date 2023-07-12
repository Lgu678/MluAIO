#pragma once
namespace taric
{
    void load();
    void unload();
    void add_circle(vector const& center, float radius, unsigned long color, float thickness = 1.f, float height = -1.f, int num_segments = 200);
    void drawCircle(vector pos, int radius, int quality, unsigned long color, int thickness); // New function declaration
    void on_draw_with_add_circle();
};