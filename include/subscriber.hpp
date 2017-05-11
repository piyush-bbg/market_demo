class subscriber
{
public:
  virtual void update(const char* msg, size_t len) = 0;
};
