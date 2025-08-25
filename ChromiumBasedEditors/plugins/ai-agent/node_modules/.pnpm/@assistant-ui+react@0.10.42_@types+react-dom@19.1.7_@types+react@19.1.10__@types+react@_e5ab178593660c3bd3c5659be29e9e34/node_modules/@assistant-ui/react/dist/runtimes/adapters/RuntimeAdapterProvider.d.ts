import { FC, ReactNode } from "react";
import { ThreadHistoryAdapter } from "./thread-history/ThreadHistoryAdapter";
import { ModelContextProvider } from "../../model-context";
export type RuntimeAdapters = {
    modelContext?: ModelContextProvider;
    history?: ThreadHistoryAdapter;
};
declare namespace RuntimeAdapterProvider {
    type Props = {
        adapters: RuntimeAdapters;
        children: ReactNode;
    };
}
export declare const RuntimeAdapterProvider: FC<RuntimeAdapterProvider.Props>;
export declare const useRuntimeAdapters: () => RuntimeAdapters | null;
export {};
//# sourceMappingURL=RuntimeAdapterProvider.d.ts.map