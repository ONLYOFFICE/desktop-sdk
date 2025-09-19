"use client";

import { type FC } from "react";
import ShikiHighlighter, {
  type ShikiHighlighterProps,
  useShikiHighlighter,
} from "react-shiki";
import type { SyntaxHighlighterProps as AUIProps } from "@assistant-ui/react-markdown";
import { cn } from "@/lib/utils";

import "./styles.css";

/**
 * Props for the SyntaxHighlighter component
 */
export type HighlighterProps = Omit<
  ShikiHighlighterProps,
  "children" | "theme"
> & {
  theme?: ShikiHighlighterProps["theme"];
} & Pick<AUIProps, "node" | "components" | "language" | "code">;

/**
 * SyntaxHighlighter component, using react-shiki
 * Use it by passing to `defaultComponents` in `markdown-text.tsx`
 *
 * @example
 * const defaultComponents = memoizeMarkdownComponents({
 *   SyntaxHighlighter,
 *   h1: //...
 *   //...other elements...
 * });
 */
export const SyntaxHighlighter: FC<HighlighterProps> = ({
  code,
  language,
  theme = { dark: "kanagawa-wave", light: "kanagawa-lotus" },
  className,
  addDefaultStyles = true, // assistant-ui requires custom base styles
  showLanguage = false, // assistant-ui/react-markdown handles language labels
  ...props
}) => {
  const BASE_STYLES =
    "aui-shiki-base [&_pre]:overflow-x-auto [&_pre]:rounded-b-lg [&_pre]:!bg-muted/75 [&_pre]:p-4";

  const testCode = useShikiHighlighter(code, language, theme);

  if (!testCode) {
    return (
      <div className="px-[12px] pb-[12px] bg-[var(--chat-message-code-block-background-color)] rounded-b-[12px]">
        <pre
          className={cn(
            "overflow-x-auto bg-[var(--chat-message-code-block-pre-background-color)] border border-[var(--chat-message-code-block-border-color)] rounded-[4px] p-[6px]",
            className
          )}
        >
          {code.trim()}
        </pre>
      </div>
    );
  }

  return (
    <ShikiHighlighter
      {...props}
      language={language}
      theme={theme}
      addDefaultStyles={addDefaultStyles}
      showLanguage={showLanguage}
      defaultColor="light-dark()"
      className={cn(BASE_STYLES, className)}
    >
      {code.trim()}
    </ShikiHighlighter>
  );
};

SyntaxHighlighter.displayName = "SyntaxHighlighter";
