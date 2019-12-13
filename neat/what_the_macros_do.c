void loop(void)
{

    { Transform *transform; Iterator iterator; Manager *manager = manager_of_type(Transform_TYPE_ID); init_iterator(&iterator, manager->aspect_iterator); iterator.data1.ptr_val = manager; while (1) { step(&iterator); if (iterator.val ==
# 151 "src/rendering/rendering.c" 3 4
   ((void *)0)
# 151 "src/rendering/rendering.c"
   ) { break; } transform = (Transform *) iterator.val;
        transform->theta_x += dt() * 3;
        transform->z += dt() * 3;
        transform->y += dt() * 3;
        transform->x += dt() * 3;
    } }

    { Camera *camera; Iterator iterator; Manager *manager = manager_of_type(Camera_TYPE_ID); init_iterator(&iterator, manager->aspect_iterator); iterator.data1.ptr_val = manager; while (1) { step(&iterator); if (iterator.val ==
# 158 "src/rendering/rendering.c" 3 4
   ((void *)0)
# 158 "src/rendering/rendering.c"
   ) { break; } camera = (Camera *) iterator.val;
        Matrix4x4f vp_matrix = camera->projection_matrix;
        Matrix4x4f view_matrix = Transform_matrix(((Transform *) _get_aspect_type(( camera )->entity_id, Transform_TYPE_ID)));
        right_multiply_matrix4x4f(&vp_matrix, &view_matrix);

        { Body *body; Iterator iterator; Manager *manager = manager_of_type(Body_TYPE_ID); init_iterator(&iterator, manager->aspect_iterator); iterator.data1.ptr_val = manager; while (1) { step(&iterator); if (iterator.val ==
# 163 "src/rendering/rendering.c" 3 4
       ((void *)0)
# 163 "src/rendering/rendering.c"
       ) { break; } body = (Body *) iterator.val;
            Mesh *mesh = ( (Mesh *) ___resource_data( &( body->mesh ) ) );
            GraphicsProgram *graphics_program = ( (GraphicsProgram *) ___resource_data( &( ( (Artist *) ___resource_data( &( body->artist ) ) )->graphics_program ) ) );
            g_mvp_matrix = vp_matrix;
            Matrix4x4f model_matrix = Transform_matrix(((Transform *) _get_aspect_type(( body )->entity_id, Transform_TYPE_ID)));
            right_multiply_matrix4x4f(&g_mvp_matrix, &model_matrix);
            Artist_draw_mesh(( (Artist *) ___resource_data( &( body->artist ) ) ), mesh);
        } }
    } }
# 187 "src/rendering/rendering.c"
}

