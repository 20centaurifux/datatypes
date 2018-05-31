(ns benchmark.core
  (:require [clojure.java.io :as io]))

(defn- scanner
  [path]
  (doto
    (java.util.Scanner. (io/as-file path))
    (.useDelimiter " ")))

(defn- scan-words
  [scanner]
  (lazy-seq
    (when (.hasNext scanner)
      (cons (.next scanner) (scan-words scanner)))))

(defn- count-words
  [path]
  (-> (scanner path) scan-words frequencies))

(defn benchmark
  []
  (time
    (when-let [m (count-words "words.txt")]
      (println (count m))
      (println (reduce + (vals m))))))
