// QuerySet.h
#pragma once

/// A simple C++ wrapper around a WGPUQuerySet.
/// You can use this to create a timestamp or pipeline‑statistics query set
/// and retrieve its handle and count.
class QuerySet
{
private:
   WGPUQuerySet _querySet;
   uint32_t _count;

public:
   /// \param device   A pointer to your Device wrapper (must expose a method Get() → WGPUDevice).
   /// \param type     The WGPUQueryType (e.g. WGPUQueryType_Timestamp).
   /// \param count    The number of query slots to allocate.
   QuerySet(Device *device, WGPUQueryType type, uint32_t count);

   ~QuerySet();

   /// Returns the underlying WGPUQuerySet handle.
   [[nodiscard]] WGPUQuerySet Get() const { return _querySet; }

   /// Returns how many query slots were allocated.
   [[nodiscard]] uint32_t GetCount() const { return _count; }

   /// Writes a timestamp to the specified query index.
   void WriteTimestamp(const CommandEncoder *encoder, uint32_t queryIndex) const;

   /// Resolve query set results to a buffer.
   void Resolve(const CommandEncoder *encoder, Buffer *buffer, uint32_t bufferOffset) const;
};
