import React from "react";
import { useTranslation } from "react-i18next";

import useProviders from "@/store/useProviders";

import { RadioButton } from "@/components/radio-button";
import { Tabs } from "@/components/tabs";

import { Providers } from "./sub-components/providers";
import { Wallet } from "./sub-components/wallet";
import { Servers } from "./sub-components/servers";

import config from "@/config.json";

const showWallet = config.showWallet;

const Settings = () => {
  const { t } = useTranslation();

  const [selectedSection, setSelectedSection] = React.useState(
    showWallet ? "wallet" : "providers"
  );

  const { providers } = useProviders();

  const aiSettingsTab = (
    <div className="flex flex-col gap-[16px] select-none">
      {showWallet ? (
        <div>
          <h3 className="font-bold text-[20px] leading-[28px] text-[var(--settings-header-color)]">
            {t("ChooseHowConnect")}
          </h3>
          <p className="text-[14px] leading-[20px] text-[var(--settings-description-color)] mt-[4px]">
            {t("SelectHowConnect")}
            <br />
            {t("SelectHowConnectDescription")}
          </p>
        </div>
      ) : null}
      {(showWallet ? ["wallet", "providers"] : ["providers"]).map((item) => {
        const isWallet = item === "wallet";

        return (
          <div key={item} className="flex gap-[12px]">
            {showWallet ? (
              <div className="flex items-start w-[20px] flex-shrink-0">
                <RadioButton
                  checked={selectedSection === item}
                  onChange={() => setSelectedSection(item)}
                />
              </div>
            ) : null}
            <div className="select-none flex flex-col gap-[12px]">
              <div className="flex flex-col gap-[4px]">
                <h2
                  className="font-normal text-[14px] leading-[20px] text-[var(--text-normal)] cursor-pointer"
                  onClick={() => setSelectedSection(item)}
                >
                  {isWallet ? t("ONLYOFFICEWallet") : t("AIProviders")}
                </h2>
                <p className="text-[14px] leading-[20px] text-[var(--settings-description-color)] mt-[4px]">
                  {isWallet
                    ? t("ONLYOFFICEWalletDescription")
                    : t("AIProvidersDescription")}
                </p>
              </div>
              {isWallet ? (
                <Wallet isActive={selectedSection === item} />
              ) : (
                <Providers isActive={selectedSection === item} />
              )}
            </div>
          </div>
        );
      })}
    </div>
  );

  return (
    <>
      <div className="flex justify-center">
        <div className="flex flex-col gap-[16px] box-border max-w-[640px] w-[640px] mt-[32px]">
          <h1 className="select-none font-bold text-[20px] leading-[28px] text-[var(--settings-header-color)]">
            {t("Settings")}
          </h1>
          <Tabs
            items={[
              {
                value: "ai-settings",
                label: t("Connection"),
                content: aiSettingsTab,
              },
              {
                value: "mcp-servers",
                label: t("MCPServers"),
                content: <Servers />,
                disabled: !providers.length,
              },
            ]}
          />
        </div>
      </div>
    </>
  );
};

export default Settings;
