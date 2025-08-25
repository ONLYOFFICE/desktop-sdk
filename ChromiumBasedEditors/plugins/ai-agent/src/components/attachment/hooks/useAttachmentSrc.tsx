import { useShallow } from "zustand/shallow";
import { useAttachment } from "@assistant-ui/react";

import useFileSrc from "./useFileSrc";

const useAttachmentSrc = () => {
  const { file, src } = useAttachment(
    useShallow((a): { file?: File; src?: string } => {
      if (a.type !== "image") return {};
      if (a.file) return { file: a.file };
      const src = a.content?.filter((c) => c.type === "image")[0]?.image;
      if (!src) return {};
      return { src };
    })
  );

  return useFileSrc(file) ?? src;
};

export default useAttachmentSrc;
