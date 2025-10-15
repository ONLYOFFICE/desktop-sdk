import React from "react";

import { Button } from "@/components/button";

import ConfigDialog from "./ConfigDialog";
import AvailableTools from "./AvailableTools";

const Servers = () => {
  const [isOpen, setIsOpen] = React.useState(false);

  return (
    <>
      <div className="flex flex-col gap-[16px] mt-[16px]">
        <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-description-color)]">
          This section allows you to configure MCP servers for use within the AI
          Agent. You can edit the configuration to add new MCP servers and
          enable tools as needed.
        </p>
        <Button className="w-fit" onClick={() => setIsOpen(true)}>
          Edit configuration
        </Button>
        <AvailableTools />
      </div>
      {<ConfigDialog open={isOpen} onClose={() => setIsOpen(false)} />}
    </>
  );
};

export { Servers };
