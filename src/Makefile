DEPDIR = .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
TGT=obj
$(shell mkdir -p $(TGT) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

SRC=.
TGT=obj
EXEC=BreakID
INCLUDES = -Iinclude
CXXFLAGS = $(DEPFLAGS) -Wall -std=c++11 -O3  $(INCLUDES)  
LDFLAGS =  -static
BOOSTUSED =  
SOURCES = $(wildcard $(SRC)/*.cpp $(SRC)/*.c)
HEADERS = $(wildcard $(SRC)/*.hpp $(SRC)/*.h)
DEPFILES = $(wildcard $(DEPDIR)/*.d)
FAKESOURCES=$(SOURCES:.c=.cpp)
OBJS = $(addprefix $(TGT)/, $(notdir $(FAKESOURCES:.cpp=.o)))
GIT_VERSION := $(shell sh -c 'git describe --tags --long --dirty')
CXXFLAGS += -DVERSION=\"$(GIT_VERSION)\"
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d




all: CXXFLAGS += -DNDEBUG -ggdb3
all: $(EXEC) $(TGT)/.compiler_flags

.PHONY: force
$(TGT)/.compiler_flags: force
	echo '$(CXXFLAGS)' | cmp -s - $@ || echo '$(CXXFLAGS)' > $@
	
debug: CXXFLAGS += -DDEBUG -g 
debug: $(EXEC)




$(TGT)/%.o: $(SRC)/%.cpp 
$(TGT)/%.o: $(SRC)/%.cpp $(DEPDIR)/%.d $(TGT)/.compiler_flags
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE) 

$(TGT)/%.o: $(SRC)/%.c
$(TGT)/%.o: $(SRC)/%.c $(DEPDIR)/%.d $(TGT)/.compiler_flags
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE) 



$(EXEC): $(OBJS) $(TGT)/.compiler_flags
	echo $(OBJS); $(CXX) $(LDFLAGS) $(OBJS) -o $@ $(BOOSTUSED)

	
clean: 
	rm $(TGT)/*.o; rm $(EXEC); rm -rf .d; rm -rf $(TGT)
	
$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

-include $(DEPFILES)	










