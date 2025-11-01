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
    }, 1000);
  }, [isReady, initServers, getTools]);

  useEffect(() => {
    if (!tools || !currentProvider) return;

    provider.setCurrentProviderTools(tools);
  }, [tools, currentProvider]);

  return {};
};

export default useServers;
