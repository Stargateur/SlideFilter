#include <obs-module.h>

OBS_DECLARE_MODULE()

#include <float.h>

#define SLIDE_FILTER_NAME "SlideFilter"

struct slide_filter_data {
  obs_source_t *context;
  float time_remaining;
  float interval;
  size_t active_source;
  size_t current_source;
  bool random;
};

static const char *slide_filter_get_name(void *unused) {
  UNUSED_PARAMETER(unused);
  return obs_module_text(SLIDE_FILTER_NAME);
}

static void slide_filter_destroy(void *data) {
  struct slide_filter_data *filter = data;
  blog(LOG_ERROR, "free data " SLIDE_FILTER_NAME);
  free(filter);
}

static void slide_filter_remove_callback(obs_source_t *parent,
                                         obs_source_t *child, void *data) {
  struct slide_filter_data *filter = data;
  blog(LOG_ERROR, "child scene name, %s", obs_source_get_name(child));
  obs_source_set_enabled(child, true);
}

static void slide_filter_remove(void *data, obs_source_t *source) {
  struct slide_filter_data *filter = data;
  if (!source) {
    return;
  }
  blog(LOG_ERROR, "Scene name: %s", obs_source_get_name(source));
  obs_source_enum_active_sources(source, &slide_filter_remove_callback, filter);
}

static void slide_filter_update(void *data, obs_data_t *settings) {
  struct slide_filter_data *filter = data;

  filter->interval = (float)obs_data_get_double(settings, "interval");
  filter->random = obs_data_get_bool(settings, "random");
}

static obs_properties_t *slide_filter_properties(void *data) {
  obs_properties_t *props = obs_properties_create();

  obs_properties_add_float(props, "interval",
                           obs_module_text("Interval between change of source"),
                           0.01, FLT_MAX, 0.01);

  obs_properties_add_bool(props, "random",
                          obs_module_text("Randomize the slide"));

  UNUSED_PARAMETER(data);
  return props;
}

static void slide_filter_defaults(obs_data_t *settings) {
  obs_data_set_default_double(settings, "interval", 1);
  obs_data_set_default_bool(settings, "random", false);
}

static void *slide_filter_create(obs_data_t *settings, obs_source_t *context) {
  struct slide_filter_data *filter = malloc(sizeof *filter);
  if (!filter) {
    blog(LOG_ERROR, "No memory");
    return NULL;
  }
  slide_filter_defaults(settings);
  slide_filter_update(filter, settings);

  filter->time_remaining = filter->interval;
  filter->context = context;
  filter->active_source = 0;

  return filter;
}

static void slide_filter_tick(void *data, float seconds) {
  struct slide_filter_data *filter = data;

  filter->time_remaining += seconds;
}

static void slide_filter_render_callback(obs_source_t *parent,
                                         obs_source_t *child, void *data) {
  struct slide_filter_data *filter = data;
  blog(LOG_ERROR, "child scene name, %s", obs_source_get_name(child));
  if (filter->current_source++ == filter->active_source) {
    obs_source_set_enabled(child, true);
  } else {
    obs_source_set_enabled(child, false);
  }
}

static void slide_filter_render(void *data, gs_effect_t *effect) {
  struct slide_filter_data *filter = data;

  while (filter->time_remaining > filter->interval) {
    obs_source_t *parent = obs_filter_get_parent(filter->context);
    if (!parent) {
      blog(LOG_ERROR, "%s need to be in a scene", SLIDE_FILTER_NAME);
      return;
    }
    blog(LOG_ERROR, "Scene name: %s", obs_source_get_name(parent));
    filter->current_source = 0;
    obs_source_enum_active_sources(parent, &slide_filter_render_callback,
                                   filter);
    if (++filter->active_source == filter->current_source) {
      filter->active_source = 0;
    }
    if (filter->random) {
      filter->active_source = rand() % filter->current_source;
      filter->current_source = 0;
      obs_source_enum_active_sources(parent, &slide_filter_render_callback,
                                     filter);
    }
    filter->time_remaining -= filter->interval;
  }

  obs_source_skip_video_filter(filter->context);

  UNUSED_PARAMETER(effect);
}

static uint32_t slide_filter_width(void *data) {
  struct slide_filter_data *filter = data;
  if (!filter) {
    blog(LOG_ERROR, SLIDE_FILTER_NAME ": slide_filter_width error");
    return 0;
  }
  obs_source_t *target = obs_filter_get_target(filter->context);

  return obs_source_get_base_width(target);
}

static uint32_t slide_filter_height(void *data) {
  struct slide_filter_data *filter = data;
  if (!filter) {
    blog(LOG_ERROR, SLIDE_FILTER_NAME ": slide_filter_height error");
    return 0;
  }
  obs_source_t *target = obs_filter_get_target(filter->context);

  return obs_source_get_base_height(target);
}

static struct obs_source_info slide_filter = {
    .id = "slide_filter",
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = slide_filter_get_name,
    .create = slide_filter_create,
    .destroy = slide_filter_destroy,
    .update = slide_filter_update,
    .get_properties = slide_filter_properties,
    .get_defaults = slide_filter_defaults,
    .video_tick = slide_filter_tick,
    .video_render = slide_filter_render,
    .filter_remove = slide_filter_remove,
    .get_width = slide_filter_width,
    .get_height = slide_filter_height};

bool obs_module_load(void) {
  blog(LOG_ERROR, "coucoucou");
  obs_register_source(&slide_filter);
  return true;
}

static const char *obs_module_text(const char *val) { return val; }

static bool obs_module_get_string(const char *val, const char **out) {
  *out = val;
  return true;
}

static void obs_module_set_locale(const char *locale) {}
