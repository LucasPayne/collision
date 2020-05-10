#ifndef WIDGETS_H
#define WIDGETS_H

typedef struct ControlWidget_s {
    float size;
    bool dragging;
    int dragging_plane; // yz, xz, xy
    vec3 drag_offset;
    float alpha;
} ControlWidget;

void ControlWidget_update(Logic *g);
void ControlWidget_mouse_button_listener(Logic *g, MouseButton button, bool click, float x, float y);
void ControlWidget_mouse_move_listener(Logic *g, float x, float y, float dx, float dy);
ControlWidget *ControlWidget_add(EntityID entity, float size);

#endif // WIDGETS_H
