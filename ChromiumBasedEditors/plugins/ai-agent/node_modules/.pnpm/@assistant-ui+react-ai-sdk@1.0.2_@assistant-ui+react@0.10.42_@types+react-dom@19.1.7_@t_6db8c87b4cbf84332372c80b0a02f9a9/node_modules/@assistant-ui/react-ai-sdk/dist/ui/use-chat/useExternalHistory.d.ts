import { AssistantRuntime, ThreadHistoryAdapter, ThreadMessage, MessageFormatAdapter, MessageFormatRepository, ExportedMessageRepository } from "@assistant-ui/react";
import { RefObject } from "react";
export declare const toExportedMessageRepository: <TMessage>(toThreadMessages: (messages: TMessage[]) => ThreadMessage[], messages: MessageFormatRepository<TMessage>) => ExportedMessageRepository;
export declare const useExternalHistory: <TMessage>(runtimeRef: RefObject<AssistantRuntime>, historyAdapter: ThreadHistoryAdapter | undefined, toThreadMessages: (messages: TMessage[]) => ThreadMessage[], storageFormatAdapter: MessageFormatAdapter<TMessage, any>, onSetMessages: (messages: TMessage[]) => void) => boolean;
//# sourceMappingURL=useExternalHistory.d.ts.map