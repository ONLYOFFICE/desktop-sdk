import { useState } from "react";

import type { AttachmentPreviewProps } from "../Attachment.types";

const AttachmentPreview = ({ src }: AttachmentPreviewProps) => {
  const [isLoaded, setIsLoaded] = useState(false);

  return (
    <img
      src={src}
      style={{
        width: "auto",
        height: "auto",
        maxWidth: "75dvh",
        maxHeight: "75dvh",
        display: isLoaded ? "block" : "none",
        overflow: "clip",
      }}
      onLoad={() => setIsLoaded(true)}
      alt="Preview"
    />
  );
};

export default AttachmentPreview;
