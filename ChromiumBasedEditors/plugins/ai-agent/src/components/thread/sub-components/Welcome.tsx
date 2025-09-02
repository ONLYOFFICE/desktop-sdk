import { ThreadPrimitive } from "@assistant-ui/react";

const ThreadWelcome = () => {
  return (
    <ThreadPrimitive.Empty>
      <div className="mx-auto flex w-full max-w-[var(--thread-max-width)] flex-col px-[var(--thread-padding-x)] justify-center items-center">
        <div className="max-w-[490px] flex flex-col items-center gap-[8px] text-center">
          <p
            className="font-[700] text-[17px] leading-[24px]"
            style={{ color: "var(--welcome-title-color)" }}
          >
            How can I help?
          </p>
          <p
            className="text-[14px] leading-[20px]"
            style={{ color: "var(--welcome-description-color)" }}
          >
            An AI agent in ONLYOFFICE improves productivity and user experience
            through task automation and smart suggestions across its tools.
          </p>
        </div>
      </div>
    </ThreadPrimitive.Empty>
  );
};

export { ThreadWelcome };
