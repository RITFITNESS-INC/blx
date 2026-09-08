{
  "CLASSNAME" : "view",
  "controls" : [ {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "state_label",
        "id" : 1,
        "pt" : { "x" : 90, "y" : 26 },
        "size" : { "width" : 300, "height" : 36 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "未连接",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 6714767,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 20
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "button",
    "theme" : "youth",
    "type" : 1,
    "property" : {
      "pub_t" : {
        "name" : "mode_prev_button",
        "id" : 2,
        "ctrl_type" : 1,
        "pt" : { "x" : 70, "y" : 73 },
        "size" : { "width" : 74, "height" : 42 },
        "t_mode" : 1,
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "txt" : "‹",
      "styles" : [ {
        "name" : "style_rel",
        "body_t" : { "main_color" : 1516608, "radius" : 21 },
        "txt_t" : {
          "color" : 10465484,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 30
        },
        "img_t" : { }
      }, {
        "style_state" : 32,
        "name" : "style_pr",
        "body_t" : { "main_color" : 2504792, "radius" : 21 },
        "txt_t" : { "color" : 3594239, "font_size" : 30 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "mode_label",
        "id" : 3,
        "pt" : { "x" : 145, "y" : 72 },
        "size" : { "width" : 190, "height" : 44 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "基础阻力",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 16777215,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 28
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "button",
    "theme" : "youth",
    "type" : 1,
    "property" : {
      "pub_t" : {
        "name" : "mode_next_button",
        "id" : 4,
        "ctrl_type" : 1,
        "pt" : { "x" : 336, "y" : 73 },
        "size" : { "width" : 74, "height" : 42 },
        "t_mode" : 1,
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "txt" : "›",
      "styles" : [ {
        "name" : "style_rel",
        "body_t" : { "main_color" : 1516608, "radius" : 21 },
        "txt_t" : {
          "color" : 10465484,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 30
        },
        "img_t" : { }
      }, {
        "style_state" : 32,
        "name" : "style_pr",
        "body_t" : { "main_color" : 2504792, "radius" : 21 },
        "txt_t" : { "color" : 3594239, "font_size" : 30 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "arc",
    "theme" : "youth",
    "type" : 5,
    "property" : {
      "pub_t" : {
        "name" : "value_arc",
        "id" : 5,
        "ctrl_type" : 5,
        "pt" : { "x" : 105, "y" : 112 },
        "size" : { "width" : 270, "height" : 270 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "angle_t" : { "start_angle" : 135, "end_angle" : 45 },
      "range_t" : { "maxvalue" : 500 },
      "know_dis_en" : true,
      "arc_t_main" : {
        "color" : 4280694872,
        "width" : 18,
        "opa" : 255,
        "rounded" : 10
      },
      "arc_t_inc" : {
        "color" : 4281784319,
        "width" : 18,
        "opa" : 255,
        "rounded" : 10
      },
      "styles" : [ {
        "style_part" : 196608,
        "name" : "knob_style",
        "body_t" : { "main_color" : 3594239, "grad_color" : 3594239, "radius" : -1 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "parameter_label",
        "id" : 6,
        "pt" : { "x" : 140, "y" : 151 },
        "size" : { "width" : 200, "height" : 35 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "向心力",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 10465484,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 21
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "value_label",
        "id" : 7,
        "pt" : { "x" : 115, "y" : 185 },
        "size" : { "width" : 250, "height" : 82 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "5.0",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 16777215,
          "font_lib" : "AlibabaPuHuiTi-3-95-ExtraBold.ttf",
          "font_size" : 62
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "unit_label",
        "id" : 8,
        "pt" : { "x" : 165, "y" : 263 },
        "size" : { "width" : 150, "height" : 31 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "N",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 3594239,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 22
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "hint_label",
        "id" : 9,
        "pt" : { "x" : 90, "y" : 299 },
        "size" : { "width" : 300, "height" : 28 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "按住旋转切模式 · 长按启动",
      "styles" : [ {
        "name" : "style",
        "txt_t" : {
          "color" : 6714767,
          "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf",
          "font_size" : 16
        },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "force_label",
        "id" : 10,
        "pt" : { "x" : 45, "y" : 337 },
        "size" : { "width" : 135, "height" : 30 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "拉力 --.- N",
      "styles" : [ {
        "name" : "style",
        "txt_t" : { "color" : 10465484, "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf", "font_size" : 17 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "speed_label",
        "id" : 11,
        "pt" : { "x" : 180, "y" : 337 },
        "size" : { "width" : 120, "height" : 30 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "速度 --",
      "styles" : [ {
        "name" : "style",
        "txt_t" : { "color" : 10465484, "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf", "font_size" : 17 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "label",
    "theme" : "youth",
    "property" : {
      "pub_t" : {
        "name" : "position_label",
        "id" : 12,
        "pt" : { "x" : 300, "y" : 337 },
        "size" : { "width" : 135, "height" : 30 },
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "animspeed" : 50,
      "align_e" : 34,
      "txt" : "位置 --",
      "styles" : [ {
        "name" : "style",
        "txt_t" : { "color" : 10465484, "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf", "font_size" : 17 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "button",
    "theme" : "youth",
    "type" : 1,
    "property" : {
      "pub_t" : {
        "name" : "parameter_button",
        "id" : 13,
        "ctrl_type" : 1,
        "pt" : { "x" : 76, "y" : 391 },
        "size" : { "width" : 150, "height" : 48 },
        "t_mode" : 1,
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "txt" : "切换参数",
      "styles" : [ {
        "name" : "style_rel",
        "body_t" : { "main_color" : 1516608, "radius" : 24 },
        "txt_t" : { "color" : 16777215, "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf", "font_size" : 20 },
        "img_t" : { }
      }, {
        "style_state" : 32,
        "name" : "style_pr",
        "body_t" : { "main_color" : 2504792, "radius" : 24 },
        "txt_t" : { "color" : 3594239, "font_size" : 20 },
        "img_t" : { }
      } ]
    }
  }, {
    "CLASSNAME" : "button",
    "theme" : "youth",
    "type" : 1,
    "property" : {
      "pub_t" : {
        "name" : "run_button",
        "id" : 14,
        "ctrl_type" : 1,
        "pt" : { "x" : 254, "y" : 391 },
        "size" : { "width" : 150, "height" : 48 },
        "t_mode" : 1,
        "scrollbar_t" : { },
        "alpha" : 255,
        "pub" : true
      },
      "txt" : "启动",
      "styles" : [ {
        "name" : "style_rel",
        "body_t" : { "main_color" : 5825435, "radius" : 24 },
        "txt_t" : { "color" : 725024, "font_lib" : "HarmonyOS_Sans_SC_Regular.ttf", "font_size" : 21 },
        "img_t" : { }
      }, {
        "style_state" : 32,
        "name" : "style_pr",
        "body_t" : { "main_color" : 3594239, "radius" : 24 },
        "txt_t" : { "color" : 725024, "font_size" : 21 },
        "img_t" : { }
      } ]
    }
  } ],
  "view" : {
    "v_name" : "tension",
    "v_id" : 1,
    "bg_color" : 4278915104
  }
}
