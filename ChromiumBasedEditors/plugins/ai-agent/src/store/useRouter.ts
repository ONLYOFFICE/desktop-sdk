import { create } from "zustand";

export type Page = "chat" | "settings";

type RouterState = {
  currentPage: Page;
  setCurrentPage: (page: Page) => void;
  goToChat: () => void;
  goToSettings: () => void;
};

const useRouter = create<RouterState>((set) => ({
  currentPage: "chat",
  setCurrentPage: (page: Page) => set({ currentPage: page }),
  goToChat: () => set({ currentPage: "chat" }),
  goToSettings: () => set({ currentPage: "settings" }),
}));

export default useRouter;
