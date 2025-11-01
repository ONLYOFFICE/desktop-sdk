import * as TabsPrimitive from "@radix-ui/react-tabs";
import { cn } from "@/lib/utils";
import type { TabsProps } from "./Tabs.types";

const Tabs = ({
  items,
  defaultValue,
  value,
  onValueChange,
  className,
}: TabsProps) => {
  return (
    <TabsPrimitive.Root
      defaultValue={defaultValue || items[0]?.value}
      value={value}
      onValueChange={onValueChange}
      className={cn("w-full", className)}
    >
      <TabsPrimitive.List className="w-full inline-flex h-[32px] gap-[20px] items-center border-b-[1px] border-[var(--tabs-border-color)]">
        {items.map((item) => (
          <TabsPrimitive.Trigger
            key={item.value}
            value={item.value}
            disabled={item.disabled}
            className={cn(
              "font-normal text-[14px] leading-[20px] text-[var(--tabs-color)] cursor-pointer pb-[3px] relative",
              "hover:enabled:data-[state=inactive]:after:content-[''] hover:enabled:data-[state=inactive]:after:absolute hover:enabled:data-[state=inactive]:after:bottom-[-5px] hover:enabled:data-[state=inactive]:after:left-1/2 hover:enabled:data-[state=inactive]:after:transform hover:enabled:data-[state=inactive]:after:-translate-x-1/2 hover:enabled:data-[state=inactive]:after:w-[24px] hover:enabled:data-[state=inactive]:after:h-[4px] hover:enabled:data-[state=inactive]:after:bg-[var(--tabs-hover-border-color)] hover:enabled:data-[state=inactive]:after:rounded-t-[3px]",
              "data-[state=active]:font-bold data-[state=active]:after:content-[''] data-[state=active]:after:absolute data-[state=active]:after:bottom-[-5px] data-[state=active]:after:left-0 data-[state=active]:after:right-0 data-[state=active]:after:h-[4px] data-[state=active]:after:bg-[var(--tabs-active-border-color)] data-[state=active]:after:rounded-t-[3px]",
              "disabled:opacity-50 disabled:cursor-not-allowed"
            )}
          >
            {item.label}
          </TabsPrimitive.Trigger>
        ))}
      </TabsPrimitive.List>
      {items.map((item) => (
        <TabsPrimitive.Content
          key={item.value}
          value={item.value}
          className="mt-[24px]"
        >
          {item.content}
        </TabsPrimitive.Content>
      ))}
    </TabsPrimitive.Root>
  );
};

export { Tabs };
