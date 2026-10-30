#include "data_writer.h"

#include "errors.h"

DataWriter::DataWriter(const std::string& path)
    : path(path)
    , file(path)
{
    if (!file)
        throw IOError("DataWriter::DataWriter",
                      "cannot open output file '" + path + "'");
}

void DataWriter::write(const Results& results)
{
    file << results.size();
    for (const Value result : results)
        file << ' ' << result;
    file << '\n';
    file.flush();
    if (!file)
        throw IOError("DataWriter::write",
                      "cannot write results to '" + path + "'");
}
