import React from "react";

import client from "@/servers";

import { Button } from "@/components/button";
import { ComboBox } from "@/components/combo-box";
import { Input } from "@/components/input";
import { FieldContainer } from "@/components/field-container";

const WebSearch = () => {
  const [selectedProvider, setSelectedProvider] = React.useState<string>("");
  const [apiKey, setApiKey] = React.useState<string>("");
  const [saved, setSaved] = React.useState<boolean>(false);

  React.useEffect(() => {
    const data = client.getWebSearchData();

    if (data) {
      setSelectedProvider(data.provider);
      setApiKey(data.key);
      setSaved(true);
    }
  }, []);

  const saveWebSearchData = () => {
    if (!selectedProvider || !apiKey) return;
    client.setWebSearchData({
      provider: selectedProvider,
      key: apiKey,
    });
    setSaved(true);
  };

  const resetSettings = () => {
    setSelectedProvider("");
    setApiKey("");
    setSaved(false);
    client.setWebSearchData(null);
  };

  return (
    <>
      <div className="flex flex-col gap-[16px] mt-[16px]">
        <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-description-color)]">
          Connect a web search engine to enhance AI chats with real-time
          information from the internet.
        </p>
        <div className="flex flex-col gap-[16px]">
          <FieldContainer header={"Web Search Engine"}>
            <ComboBox
              className="w-[260px]"
              value={selectedProvider || "Select provider"}
              items={[
                {
                  text: "Exa",
                  id: "exa",
                  onClick: () => setSelectedProvider("exa"),
                },
              ]}
            />
          </FieldContainer>
          <FieldContainer header={"API Key"}>
            <Input
              className="w-[260px]"
              type="password"
              value={apiKey}
              onChange={(e) => setApiKey(e.target.value)}
            />
          </FieldContainer>
        </div>
        <div className="flex flex-row gap-[8px]">
          <Button className="w-fit" onClick={saveWebSearchData}>
            Save
          </Button>
          <Button
            className="w-fit"
            disabled={!saved}
            onClick={resetSettings}
            variant="default"
          >
            Reset settings
          </Button>
        </div>
      </div>
    </>
  );
};

export { WebSearch };
