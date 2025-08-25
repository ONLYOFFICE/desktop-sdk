import type { JSONSchema7 } from "json-schema";
export declare const frontendTools: (tools: Record<string, {
    description?: string;
    parameters: JSONSchema7;
}>) => {
    [k: string]: {
        inputSchema: import("ai").Schema<unknown>;
        description?: string;
    };
};
//# sourceMappingURL=frontendTools.d.ts.map