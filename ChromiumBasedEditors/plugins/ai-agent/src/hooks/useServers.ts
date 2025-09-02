import { useEffect } from "react";

import useServersStore from "@/store/useServersStore";

type UseServersProps = {
  isReady: boolean;
};

const useServers = ({ isReady }: UseServersProps) => {
  const { initServers } = useServersStore();

  useEffect(() => {
    if (!isReady) return;

    initServers();
  }, [isReady, initServers]);

  return {};
};

export default useServers;
