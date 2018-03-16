
import pandas as pd
from malss import MALSS
from sklearn.model_selection import LeaveOneOut
#一つずつ除外して実行


data = pd.read_csv('combine.csv', index_col='出席簿番号', header=0, encoding="SHIFT-JIS")

y = data['クラス']
del data['クラス']

list = []
#print(str(list))

header = data.columns
if(len(list) != 0):
    for x in range(0,len(header)):
        if (x not in list):
            del data[header[x]]

cls = MALSS('classification',
            shuffle=True, standardize=True, n_jobs=-1, cv=5,
            random_state=0, lang='jp', verbose=False).fit(data, y, algorithm_selection_only=True)
cls.remove_algorithm(5)#N
cls.remove_algorithm(4)#DT
cls.remove_algorithm(3)#LR
cls.remove_algorithm(2)#SVM(LK)
cls.remove_algorithm(1)#RandomForest
#cls.remove_algorithm(0)#SVM(RBF)
#outputname = 'result_classification'
outputname = None
cls.fit(data, y, outputname)
#print("best_score: "+str(cls.best_score))
#print("algorithms: "+str(cls.algorithms[cls.best_index].name))
#print("best_index: "+str(cls.best_index))
f = open('result.txt', 'w') # 書き込みモードで開く
f.write(str(cls.best_score)+"\n")
#f.write(str(list)+"\n")
f.write(str(cls.algorithms[cls.best_index].name)+"\n")
#f.write(str(cls.best_index)+"\n")
f.close() # ファイルを閉じる
exit()
