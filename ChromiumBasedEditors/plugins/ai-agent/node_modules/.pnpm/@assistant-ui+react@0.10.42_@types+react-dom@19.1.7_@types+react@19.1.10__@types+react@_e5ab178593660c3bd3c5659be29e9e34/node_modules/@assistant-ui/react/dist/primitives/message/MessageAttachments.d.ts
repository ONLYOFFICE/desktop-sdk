import { ComponentType, type FC } from "react";
export declare namespace MessagePrimitiveAttachments {
    type Props = {
        components: {
            Image?: ComponentType | undefined;
            Document?: ComponentType | undefined;
            File?: ComponentType | undefined;
            Attachment?: ComponentType | undefined;
        } | undefined;
    };
}
export declare const MessagePrimitiveAttachments: FC<MessagePrimitiveAttachments.Props>;
//# sourceMappingURL=MessageAttachments.d.ts.map