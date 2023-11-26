#pragma once

#define COUNT(sequence) static_cast<uint32_t>(sequence.size())
#define BYTE_SIZE(sequence) static_cast<VkDeviceSize>(sizeof(sequence[0]) * sequence.size())
#define offsetOf(s,m) static_cast<uint32_t>(offsetof(s, m))