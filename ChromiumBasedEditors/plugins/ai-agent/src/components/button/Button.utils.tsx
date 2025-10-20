import { cva } from "class-variance-authority";

import { cn } from "@/lib/utils";

// Base styles
const baseStyles = "inline-flex items-center justify-center rounded-[4px]";
const focusStyles =
  "outline-none focus:outline-none focus-visible:outline-none";

// Default variant styles
const defaultBaseStyles =
  "bg-[var(--button-default-background-color)] border border-[var(--button-default-border-color)] text-[var(--button-default-color)]";
const defaultHoverStyles =
  "hover:enabled:bg-[var(--button-default-hover-background-color)]";
const defaultActiveStyles =
  "active:enabled:bg-[var(--button-default-pressed-background-color)]";
const defaultFocusStyles =
  "focus:enabled:bg-[var(--button-default-focus-background-color)] focus:shadow-[var(--button-default-focus-shadow)]";
const defaultDisabledStyles =
  "disabled:bg-[var(--button-default-disabled-background-color)] disabled:text-[var(--button-default-disabled-color)] disabled:opacity-40 disabled:cursor-not-allowed";

// Primary variant styles
const primaryBaseStyles =
  "bg-[var(--button-background-color)] text-[var(--button-color)]";
const primaryHoverStyles =
  "hover:enabled:bg-[var(--button-background-hover-color)]";
const primaryActiveStyles =
  "active:enabled:bg-[var(--button-background-pressed-color)]";
const primaryFocusStyles =
  "focus:bg-[var(--button-background-color)] focus:border-[var(--button-border-focus-color)] focus:shadow-[var(--button-shadow-focus)]";
const primaryDisabledStyles =
  "disabled:bg-[var(--button-background-disabled-color)] disabled:text-[var(--button-disabled-color)] disabled:cursor-not-allowed";

// Size styles

const defaultSizeStyles =
  "h-[32px] min-w-[40px] px-[12px] text-[14px] font-semibold leading-[16px]";

const buttonVariants = cva(`${baseStyles} ${focusStyles}`, {
  variants: {
    variant: {
      default: cn(
        defaultBaseStyles,
        defaultHoverStyles,
        defaultActiveStyles,
        defaultFocusStyles,
        defaultDisabledStyles
      ),
      primary: cn(
        primaryBaseStyles,
        primaryHoverStyles,
        primaryActiveStyles,
        primaryFocusStyles,
        primaryDisabledStyles
      ),
    },
    size: {
      default: defaultSizeStyles,
    },
    scale: {
      default: "w-fit",
      scale: "w-full",
    },
  },
  defaultVariants: {
    variant: "primary",
    size: "default",
    scale: "default",
  },
});

export { buttonVariants };
