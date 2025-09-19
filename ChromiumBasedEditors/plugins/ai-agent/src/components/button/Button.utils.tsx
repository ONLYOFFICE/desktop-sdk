import { cva } from "class-variance-authority";

const buttonVariants = cva(
  "inline-flex items-center justify-center min-w-[48px] rounded-[4px] outline-none focus:outline-none focus-visible:outline-none",
  {
    variants: {
      variant: {
        default:
          "bg-[var(--button-default-background-color)] border border-[var(--button-default-border-color)] text-[var(--button-default-color)] hover:enabled:bg-[var(--button-default-hover-background-color)] active:enabled:bg-[var(--button-default-pressed-background-color)] focus:enabled:bg-[var(--button-default-focus-background-color)] disabled:bg-[var(--button-default-disabled-background-color)] disabled:text-[var(--button-default-disabled-color)] disabled:cursor-not-allowed",
        primary: `bg-[var(--button-background-color)] 
          text-[var(--button-color)] 
          hover:enabled:bg-[var(--button-background-hover-color)] 
          active:enabled:bg-[var(--button-background-pressed-color)] 
          focus:bg-[var(--button-background-color)]
          focus:border-[var(--button-border-focus-color)] focus:shadow-[var(--button-shadow-focus)] 
          disabled:bg-[var(--button-background-disabled-color)] disabled:text-[var(--button-disabled-color)] disabled:cursor-not-allowed`,
      },
      size: {
        small:
          "h-[24px] px-[8px] text-[12px] font-semibold leading-[16px] m-w-[64px]",
        default: "h-[32px] px-[12px] text-[14px] font-semibold leading-[16px]",
        normal: "h-[42px] px-[16px] text-[16px] font-bold leading-[24px]",
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
  }
);

export { buttonVariants };
