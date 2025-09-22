import { create } from "zustand";

import type { TAttachmentFile } from "@/lib/types";

type UseAttachmentsStoreProps = {
  attachmentFiles: TAttachmentFile[];

  addAttachmentFile: (file: TAttachmentFile) => void;
  deleteAttachmentFile: (path: string) => void;
  clearAttachmentFiles: () => void;
};

const useAttachmentsStore = create<UseAttachmentsStoreProps>((set, get) => ({
  attachmentFiles: [],

  addAttachmentFile: (file: TAttachmentFile) => {
    set({ attachmentFiles: [...get().attachmentFiles, file] });
  },
  deleteAttachmentFile: (path: string) => {
    set({
      attachmentFiles: get().attachmentFiles.filter((f) => f.path !== path),
    });
  },
  clearAttachmentFiles: () => {
    set({ attachmentFiles: [] });
  },
}));

export default useAttachmentsStore;
