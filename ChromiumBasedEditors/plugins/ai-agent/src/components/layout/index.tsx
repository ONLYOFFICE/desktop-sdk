import React from "react";
import { useTranslation } from "react-i18next";

import { Navigation } from "./sub-components/Header";
import { ChatList } from "./sub-components/ChatList";

const Layout = ({ children }: { children: React.ReactNode }) => {
  const [theme, setTheme] = React.useState(
    `theme-${window.RendererProcessVariable?.theme.type ?? "light"}`
  );

  const { i18n } = useTranslation();

  React.useLayoutEffect(() => {
    if (window.RendererProcessVariable) {
      const lang =
        window.RendererProcessVariable.lang ??
        new URLSearchParams(window.location.search).get("lang");
      if (lang?.includes("en")) {
        i18n.changeLanguage("en");
      } else if (lang?.includes("ru")) {
        i18n.changeLanguage("ru");
      } else {
        i18n.changeLanguage("en");
      }
      setTheme(`theme-${window.RendererProcessVariable.theme.type}`);
    } else {
      i18n.changeLanguage("en");
      setTheme("theme-light");
    }

    window.on_update_plugin_info = (info) => {
      if (info?.lang?.includes("en")) {
        i18n.changeLanguage("en");
      } else if (info?.lang?.includes("ru")) {
        i18n.changeLanguage("ru");
      } else {
        i18n.changeLanguage("en");
      }
      if (
        info?.theme === "theme-white" ||
        info?.theme === "theme-gray" ||
        info?.theme === "theme-classic-light" ||
        info?.theme === "theme-light"
      ) {
        setTheme("theme-light");
      } else {
        setTheme("theme-dark");
      }
    };
  }, [i18n]);

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
          <ChatList />
          <div className="w-full">{children}</div>
        </div>
      </main>
    </div>
  );
};

export { Layout };
