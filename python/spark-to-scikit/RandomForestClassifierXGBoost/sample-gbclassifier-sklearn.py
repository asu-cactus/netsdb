from sklearn.ensemble import GradientBoostingClassifier
import pandas as pd
from matplotlib import pyplot as plt
from sklearn import tree
import graphviz
import sklearn_json as skljson
from sklearn.datasets import make_hastie_10_2

# df = pd.read_csv('HIGGS_100.csv/part-00000-758749fa-cbbd-473e-b1cd-4d06fd714edd-c000.csv')
# model = GradientBoostingClassifier(n_estimators=10)
# labels = df['label']
# df = df.drop('label', axis=1)
# model.fit(df, labels)

X, y = make_hastie_10_2(random_state=0)
X_train, X_test = X[:2000], X[2000:]
y_train, y_test = y[:2000], y[2000:]
model = GradientBoostingClassifier(n_estimators=9, learning_rate=1.0, max_depth=1, random_state=0, init='zero').fit(X_train, y_train)
print (model.score(X_test, y_test))

dot_data = tree.export_graphviz(model.estimators_[0,0], out_file=None)
graph = graphviz.Source(dot_data)
graph.render("iris")
skljson.to_json(model, 'sample-gbclassifier-sklearn.json')
# fig = plt.figure(figsize=(25,20))
# tree.plot_tree(model, filled=True)
#         plt.savefig('metadata/scikit_tree_image_' + str(idx) + '.png', dpi=300)

# tree = model.estimators_[42, 0]

# # Visualization
# # Install graphviz: https://www.graphviz.org/download/
# from pydotplus import graph_from_dot_data
# from IPython.display import Image
# dot_data = export_graphviz(
#     sub_tree_42,
#     out_file=None, filled=True, rounded=True,
#     special_characters=True,
#     proportion=False, impurity=False, # enable them if you want
# )
# graph = graph_from_dot_data(dot_data)
# Image(graph.create_png())
