import { cn } from "@/lib/utils";

type LoaderProps = {
  className?: string;
  size?: number;
};

const Loader = ({ className, size = 16 }: LoaderProps) => {
  return (
    <div
      className={cn(
        "inline-block animate-spin rounded-full border-2 border-solid border-[var(--loader-border-color)] border-r-transparent align-[-0.125em] motion-reduce:animate-[spin_1.5s_linear_infinite]",
        className
      )}
      style={{
        width: `${size}px`,
        height: `${size}px`,
      }}
      role="status"
      aria-label="loading"
    />
  );
};

export { Loader };
