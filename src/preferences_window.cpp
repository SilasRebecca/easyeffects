#include "preferences_window.hpp"

namespace ui::preferences_window {

using namespace std::string_literals;

auto constexpr log_tag = "preferences_window: ";

struct _PreferencesWindow {
  AdwPreferencesWindow parent_instance{};

  GtkSwitch *enable_autostart = nullptr, *process_all_inputs = nullptr, *process_all_outputs = nullptr,
            *theme_switch = nullptr, *shutdown_on_window_close = nullptr, *use_cubic_volumes = nullptr;

  GSettings* settings = nullptr;
};

G_DEFINE_TYPE(PreferencesWindow, preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

auto on_enable_autostart(GtkSwitch* obj, gboolean state, gpointer user_data) -> gboolean {
  std::filesystem::path autostart_dir{g_get_user_config_dir() + "/autostart"s};

  if (!std::filesystem::is_directory(autostart_dir)) {
    std::filesystem::create_directories(autostart_dir);
  }

  std::filesystem::path autostart_file{g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s};

  if (state != 0) {
    if (!std::filesystem::exists(autostart_file)) {
      std::ofstream ofs{autostart_file};

      ofs << "[Desktop Entry]\n";
      ofs << "Name=EasyEffects\n";
      ofs << "Comment=EasyEffects Service\n";
      ofs << "Exec=easyeffects --gapplication-service\n";
      ofs << "Icon=easyeffects\n";
      ofs << "StartupNotify=false\n";
      ofs << "Terminal=false\n";
      ofs << "Type=Application\n";

      ofs.close();

      util::debug(log_tag + "autostart file created"s);
    }
  } else {
    if (std::filesystem::exists(autostart_file)) {
      std::filesystem::remove(autostart_file);

      util::debug(log_tag + "autostart file removed"s);
    }
  }

  return 0;
}

void preferences_window_class_init(PreferencesWindowClass* klass) {
  auto* widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(widget_class, "/com/github/wwmm/easyeffects/ui/preferences_window.ui");

  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, enable_autostart);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, process_all_inputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, process_all_outputs);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, theme_switch);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, shutdown_on_window_close);
  gtk_widget_class_bind_template_child(widget_class, PreferencesWindow, use_cubic_volumes);

  gtk_widget_class_bind_template_callback(widget_class, on_enable_autostart);
}

void preferences_window_init(PreferencesWindow* self) {
  gtk_widget_init_template(GTK_WIDGET(self));

  self->settings = g_settings_new("com.github.wwmm.easyeffects");

  g_settings_bind(self->settings, "use-dark-theme", self->theme_switch, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "process-all-inputs", self->process_all_inputs, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "process-all-outputs", self->process_all_outputs, "active", G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "shutdown-on-window-close", self->shutdown_on_window_close, "active",
                  G_SETTINGS_BIND_DEFAULT);
  g_settings_bind(self->settings, "use-cubic-volumes", self->use_cubic_volumes, "active", G_SETTINGS_BIND_DEFAULT);

  gtk_switch_set_active(self->enable_autostart,
                        static_cast<gboolean>(std::filesystem::is_regular_file(
                            g_get_user_config_dir() + "/autostart/easyeffects-service.desktop"s)));
}

auto create() -> PreferencesWindow* {
  return static_cast<PreferencesWindow*>(g_object_new(EE_TYPE_PREFERENCES_WINDOW, nullptr));
}

}  // namespace ui::preferences_window