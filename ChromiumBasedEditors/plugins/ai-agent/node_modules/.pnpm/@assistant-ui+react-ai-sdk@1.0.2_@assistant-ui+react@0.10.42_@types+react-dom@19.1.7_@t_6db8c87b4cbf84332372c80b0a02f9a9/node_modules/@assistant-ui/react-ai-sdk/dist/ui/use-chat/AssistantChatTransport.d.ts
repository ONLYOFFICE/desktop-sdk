import { AssistantRuntime } from "@assistant-ui/react";
import { DefaultChatTransport, HttpChatTransportInitOptions, UIMessage } from "ai";
export declare class AssistantChatTransport<UI_MESSAGE extends UIMessage> extends DefaultChatTransport<UI_MESSAGE> {
    private runtime;
    constructor(initOptions?: HttpChatTransportInitOptions<UI_MESSAGE>);
    setRuntime(runtime: AssistantRuntime): void;
}
//# sourceMappingURL=AssistantChatTransport.d.ts.map