#ifndef CONFIG_H
#define CONFIG_H

enum {
        car_symbol         = 'I',
        barrier_symbol     = '=',

        max_score          = 32767,

        start_pos          = 2,
        min_pos            = 1,
        max_pos            = 3,

        road_height        = 16,
        road_width         = 11,

        barrier_count      = max_pos,
        barrier_width      = road_width,

        min_speed          = 50,
        max_speed          = 75,

        car_font_color     = COLOR_BLUE,
        car_bg_color       = COLOR_BLACK,
        barrier_font_color = COLOR_RED,
        barrier_bg_color   = COLOR_BLACK
};

#endif
