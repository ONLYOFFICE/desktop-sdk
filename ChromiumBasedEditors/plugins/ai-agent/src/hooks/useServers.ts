import { useEffect } from "react";

import { provider } from "@/providers";
import useServersStore from "@/store/useServersStore";
import useProviders from "@/store/useProviders";

type UseServersProps = {
  isReady: boolean;
};

const useServers = ({ isReady }: UseServersProps) => {
  const { initServers, getTools, tools } = useServersStore();
  const { currentProvider } = useProviders();

  useEffect(() => {
    if (!isReady) return;

    initServers();
    getTools();

    setInterval(() => {
      getTools();
      // update tools every 5 minutes
    }, 1000 * 60 * 5);
  }, [isReady, initServers, getTools]);

  useEffect(() => {
    const handler = () => {
      getTools();
    };

    window.addEventListener("tools-changed", handler);

    return () => {
      window.removeEventListener("tools-changed", handler);
    };
  }, [getTools]);

  useEffect(() => {
    if (!tools || !currentProvider) return;

    provider.setCurrentProviderTools(tools);
  }, [tools, currentProvider]);

  return {};
};

export default useServers;
