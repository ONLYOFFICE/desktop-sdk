import React from "react";
import { useTranslation } from "react-i18next";

import useRouter from "@/store/useRouter";

import { Navigation } from "./sub-components/Header";
import { ChatList } from "./sub-components/ChatList";

const Layout = ({ children }: { children: React.ReactNode }) => {
  const { currentPage } = useRouter();

  const { i18n } = useTranslation();

  const [theme, setTheme] = React.useState(() => {
    if (window.RendererProcessVariable) {
      return window.RendererProcessVariable.theme.id;
    } else {
      return "theme-light";
    }
  });

  React.useLayoutEffect(() => {
    if (window.RendererProcessVariable) {
      i18n.changeLanguage("en");
    }

    window.on_update_plugin_info = (info) => {
      if (info.lang) {
        i18n.changeLanguage("en");
      }

      if (info.theme) {
        if (info.theme === "theme-system") {
          setTheme(`theme-${window.RendererProcessVariable.theme.system}`);
        } else {
          setTheme(info.theme);
        }
      }
    };
  }, [i18n]);

  const isSettings = currentPage === "settings";

  return (
    <div className={`h-dvh ${theme}`}>
      <main
        id="app"
        className="h-dvh bg-[var(--layout-background-color)] flex flex-col"
      >
        <Navigation />
        <div
          className="flex flex-row flex-1"
          style={{ height: "calc(100dvh - 56px)" }}
        >
          {!isSettings ? <ChatList /> : null}
          <div className="w-full">{children}</div>
        </div>
      </main>
    </div>
  );
};

export { Layout };
