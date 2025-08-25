import type { ContentBlockParam } from "@anthropic-ai/sdk/resources/index.mjs";
import type { CompleteAttachment } from "@assistant-ui/react";

export const convertImageAttachmentsToContent = (
  attachments: readonly CompleteAttachment[]
) => {
  const contentBlock: ContentBlockParam[] = [];

  attachments.forEach(({ content }) => {
    content.forEach((part) => {
      if (part.type === "image") {
        // Extract base64 data and mime type from data URL
        const imageData = part.image;
        const base64Data = imageData.split(",")[1];
        const mimeType = imageData.split(";")[0].split(":")[1];

        // Validate and map MIME type to Anthropic's expected types
        let validMimeType:
          | "image/jpeg"
          | "image/png"
          | "image/gif"
          | "image/webp";
        if (mimeType === "image/jpeg" || mimeType === "image/jpg") {
          validMimeType = "image/jpeg";
        } else if (mimeType === "image/png") {
          validMimeType = "image/png";
        } else if (mimeType === "image/gif") {
          validMimeType = "image/gif";
        } else if (mimeType === "image/webp") {
          validMimeType = "image/webp";
        } else {
          // Default to JPEG if unknown type
          validMimeType = "image/jpeg";
        }

        contentBlock.push({
          type: "image",
          source: {
            type: "base64",
            media_type: validMimeType,
            data: base64Data,
          },
        });
      }
    });
  });

  return contentBlock;
};
