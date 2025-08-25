import { useState, useEffect } from "react";

import type { TMCPItem } from "@/lib/types";
import Tools from "@/tools";

type UseToolsProps = {
  isReady: boolean;
};

const client = new Tools();

const useTools = ({ isReady }: UseToolsProps) => {
  const [tools, setTools] = useState<TMCPItem[]>([]);

  useEffect(() => {
    if (!isReady) return;

    client.init();
    setTools(client.getTools());
  }, [isReady]);

  const callTools = (name: string, args: Record<string, unknown>) => {
    return client.callTools(name, args);
  };

  return { tools, callTools };
};

export default useTools;
