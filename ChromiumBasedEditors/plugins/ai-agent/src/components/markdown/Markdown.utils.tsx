import "@assistant-ui/react-markdown/styles/dot.css";
import { SyntaxHighlighter } from "@/components/assistant-ui/shiki-highlighter";

import {
  unstable_memoizeMarkdownComponents as memoizeMarkdownComponents,
  useIsMarkdownCodeBlock,
} from "@assistant-ui/react-markdown";

import { cn } from "../../lib/utils";

import CodeHeader from "./sub-components/CodeHeader";

const defaultComponents = memoizeMarkdownComponents({
  SyntaxHighlighter,
  h1: ({ className, ...props }) => (
    <h1
      className={cn(
        "mb-8 scroll-m-20 text-[28px] leading-[36px] font-bold last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  h2: ({ className, ...props }) => (
    <h2
      className={cn(
        "mb-4 mt-8 scroll-m-20 text-[24px] leading-[32px] font-bold first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  h3: ({ className, ...props }) => (
    <h3
      className={cn(
        "mb-4 mt-6 scroll-m-20 text-[20px] leading-[28px] font-bold first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  h4: ({ className, ...props }) => (
    <h4
      className={cn(
        "mb-4 mt-6 scroll-m-20 text-[18px] leading-[24px] font-bold first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  h5: ({ className, ...props }) => (
    <h5
      className={cn(
        "my-4 text-[16px] leading-[22px] font-bold first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  h6: ({ className, ...props }) => (
    <h6
      className={cn(
        "my-4 text-[14px] leading-[20px] font-bold first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  p: ({ className, ...props }) => (
    <p
      className={cn(
        "mb-5 mt-5 text-[14px] font-normal leading-[20px] first:mt-0 last:mb-0 text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  a: ({ className, ...props }) => (
    <a
      className={cn(
        "text-[var(--chat-link-color)] font-medium underline underline-offset-4",
        className
      )}
      {...props}
    />
  ),
  blockquote: ({ className, ...props }) => (
    <blockquote
      className={cn("border-l-2 pl-6 italic", className)}
      {...props}
    />
  ),
  ul: ({ className, ...props }) => (
    <ul
      className={cn(
        "my-5 ml-6 list-disc [&>li]:mt-[4px] text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  ol: ({ className, ...props }) => (
    <ol
      className={cn(
        "my-5 ml-6 list-decimal [&>li]:mt-[4px] text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  hr: ({ className, ...props }) => (
    <hr
      className={cn(
        "my-5 border-b border-[var(--chat-message-divider-color)]",
        className
      )}
      {...props}
    />
  ),
  table: ({ className, ...props }) => (
    <table
      className={cn(
        "my-5 w-full border-separate border-spacing-0 overflow-y-auto text-[var(--chat-message-color)]",
        className
      )}
      {...props}
    />
  ),
  th: ({ className, ...props }) => (
    <th
      className={cn(
        "px-4 py-2 border-b border-[var(--chat-message-divider-color)] text-left text-[var(--chat-message-th-color)] text-[14px] leading-[20px] font-bold [&[align=center]]:text-center [&[align=right]]:text-right",
        className
      )}
      {...props}
    />
  ),
  td: ({ className, ...props }) => (
    <td
      className={cn(
        "border-b border-[var(--chat-message-divider-color)] text-[var(--chat-message-td-color)] text-[14px] leading-[20px] px-4 py-2 text-left [&[align=center]]:text-center [&[align=right]]:text-right",
        className
      )}
      {...props}
    />
  ),
  tr: ({ className, ...props }) => (
    <tr
      className={cn(
        "m-0 border-b border-[var(--chat-message-divider-color)]",
        className
      )}
      {...props}
    />
  ),
  sup: ({ className, ...props }) => (
    <sup
      className={cn("[&>a]:text-xs [&>a]:no-underline", className)}
      {...props}
    />
  ),
  pre: ({ className, ...props }) => (
    <div className="px-[12px] pb-[12px] bg-[var(--chat-message-code-block-background-color)] rounded-[12px]">
      <pre
        className={cn(
          "overflow-x-auto bg-[var(--chat-message-code-block-pre-background-color)] border border-[var(--chat-message-code-block-border-color)] rounded-[4px] p-[6px]",
          className
        )}
        {...props}
      />
    </div>
  ),
  code: function Code({ className, ...props }) {
    const isCodeBlock = useIsMarkdownCodeBlock();
    return (
      <code
        className={cn(!isCodeBlock && "font-bold italic", className)}
        {...props}
      />
    );
  },
  CodeHeader,
});

export default defaultComponents;
