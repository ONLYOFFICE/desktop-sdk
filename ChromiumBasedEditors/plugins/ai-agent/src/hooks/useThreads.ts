import { useEffect } from "react";

import useThreadsStore from "@/store/useThreadsStore";

type UseThreadProps = {
  isReady: boolean;
};

const useThread = ({ isReady }: UseThreadProps) => {
  const { initThreads } = useThreadsStore();

  useEffect(() => {
    if (!isReady) return;

    initThreads();
  }, [isReady, initThreads]);

  return {};
};

export default useThread;
